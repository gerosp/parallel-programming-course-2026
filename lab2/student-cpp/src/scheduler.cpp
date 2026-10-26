#include "scheduler.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>
#include "sched_mutex.hpp"
namespace sched
{

    namespace
    {
        struct WorkerContext
        {
            Scheduler *scheduler = nullptr;
            Thread *worker = nullptr;
            int id = -1;
        };

        thread_local WorkerContext current;
        struct RunAborted
        {
        };

        WorkerContext &context()
        {
            if (!current.scheduler)
                throw std::logic_error("scheduler operation outside a worker thread");
            return current;
        }
    }

    Scheduler::Scheduler() = default;

    Scheduler::~Scheduler()
    {
        abort_and_join();
    }

    void Scheduler::spawn(std::function<void()> f)
    {
        if (started)
            throw std::logic_error("spawn must be called before run");
        auto worker = std::make_unique<Thread>();
        worker->func = std::move(f);
        threads.push_back(std::move(worker));
    }

    void Scheduler::pause(Thread &worker, ThreadState state)
    {
        worker.state = state;
        yielded.release();
        worker.turn.acquire();

        if (aborted)
            throw RunAborted{};
        worker.has_touched_sync = false;
    }

    void Scheduler::before_operation(Thread &worker)
    {
        if (worker.has_touched_sync)
            pause(worker, WAIT_TURN);
        worker.has_touched_sync = true;
    }

    void Scheduler::run(Strategy &strategy)
    {
        if (started)
            throw std::logic_error("each Scheduler supports one run");
        started = true;

        try
        {
            for (std::size_t id = 0; id < threads.size(); ++id)
            {
                Thread *worker = threads[id].get();
                worker->native = std::thread([this, worker, id]
                                             {
                    current = {this, worker, static_cast<int>(id)};
                    try {
                        worker->func();
                    } catch (const RunAborted&) {
                    } catch (...) {
                        worker->failure = std::current_exception();
                    }
                    worker->state = FINISHED;

                    if (!aborted)
                        yielded.release();
                    current = {}; });
                yielded.acquire();
                if (worker->failure)
                    std::rethrow_exception(worker->failure);
            }

            int last = -1;
            for (;;)
            {
                std::vector<int> enabled;
                bool unfinished = false;
                for (std::size_t id = 0; id < threads.size(); ++id)
                {
                    Thread &worker = *threads[id];
                    if (worker.failure)
                        std::rethrow_exception(worker.failure);
                    if (worker.state == FINISHED)
                        continue;
                    unfinished = true;
                    if (worker.state == WAIT_TURN ||
                        (worker.state == WAIT_COND && worker.condition()))
                        enabled.push_back(static_cast<int>(id));
                    if (worker.state == WAIT_LOCK && worker.waiting_on->owner == -1)

                        enabled.push_back(static_cast<int>(id));
                }
                if (!unfinished)
                    break;
                if (enabled.empty())
                {
                    deadlock = "held={";
                    for (const auto &lock : locks)
                    {
                        if (lock->owner != -1)
                        {
                            if (deadlock != "held={")
                                deadlock += ", ";

                            deadlock += lock->name + ":" + static_cast<char>('A' + lock->owner);
                        }
                    }
                    deadlock += "}";
                    // deadlock = "held={}";
                    abort_and_join();
                    return;
                }

                int next = strategy.pick(enabled, last);
                if (std::find(enabled.begin(), enabled.end(), next) == enabled.end())
                    throw std::logic_error("strategy selected a disabled worker");
                trace += static_cast<char>('A' + next);
                last = next;
                threads[next]->state = RUNNING;

                threads[next]->turn.release();
                yielded.acquire();
            }
            join_workers();
        }
        catch (...)
        {
            abort_and_join();
            throw;
        }
    }

    void Scheduler::join_workers()
    {
        for (auto &worker : threads)
            if (worker->native.joinable())
                worker->native.join();
    }

    void Scheduler::abort_and_join()
    {
        aborted = true;
        for (auto &worker : threads)
            if (worker->native.joinable() && worker->state != FINISHED)
                worker->turn.release();
        join_workers();
    }

    std::string Scheduler::trace_string() const
    {
        return trace;
    }

    void before_shared_access()
    {
        auto &ctx = context();
        ctx.scheduler->before_operation(*ctx.worker);
    }

    void record_operation(const std::string &operation)
    {
        auto &ctx = context();
        if (ctx.scheduler->logging)
            ctx.scheduler->log.push_back(
                std::string(1, static_cast<char>('A' + ctx.id)) + "  " + operation);
    }

    void point()
    {
        auto &ctx = context();
        ctx.worker->has_touched_sync = true;
        record_operation("window");
    }

    void await(std::function<bool()> cond)
    {
        auto &ctx = context();
        Thread &worker = *ctx.worker;
        // Predicates must only inspect data (e.g. peek()), never yield.
        bool ready = cond();
        if (worker.has_touched_sync || !ready)
        {
            worker.condition = std::move(cond);
            ctx.scheduler->pause(worker, WAIT_COND);
            worker.condition = {};
        }
        record_operation("await done");
        worker.has_touched_sync = true;
    }

    void Scheduler::wait_for_lock(sched_mutex *m)
    {
        auto &ctx = context();
        ctx.worker->waiting_on = m;
        pause(*ctx.worker, WAIT_LOCK);
        ctx.worker->waiting_on = nullptr;
        m->owner = ctx.id;
        ctx.worker->has_touched_sync = true;
        record_operation("lock " + m->name);
    }

    void Scheduler::unlock(sched_mutex *m)
    {
        if (aborted)
            return;
        auto &ctx = context();
        pause(*ctx.worker, WAIT_TURN);
        m->owner = -1;
        ctx.worker->has_touched_sync = true;
        record_operation("unlock " + m->name);
    }

    void Scheduler::register_lock(sched_mutex *m)
    {
        locks.push_back(m);
    }

} // namespace sched
