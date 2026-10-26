#pragma once
// Этап 1: планировщик, который ведёт настоящие потоки строго по одному (передача хода через семафоры).
// Этап 2: обнаружение deadlock в главном цикле и корректное завершение застрявших потоков.
// Внутреннее устройство (поля, семафоры, вспомогательные функции для shared и sched_mutex) придумываете сами.
#include <functional>
#include <exception>
#include <memory>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

#include "strategy.hpp"

namespace sched
{
    class sched_mutex;

    enum ThreadState
    {
        RUNNING,
        WAIT_TURN,
        WAIT_LOCK,
        WAIT_COND,
        FINISHED
    };

    class Thread
    {
    public:
        ThreadState state = WAIT_TURN;
        std::function<void()> func;
        // Start with a pause required before the first shared operation.
        bool has_touched_sync = true;
        std::binary_semaphore turn{0}; // sem_A, sem_B, ...
        std::thread native;
        std::function<bool()> condition;
        std::exception_ptr failure;
        sched_mutex *waiting_on = nullptr;
    };

    class Scheduler
    {
    public:
        Scheduler();
        ~Scheduler();
        // Зарегистрировать поток. Потоки получают буквы A, B, C… в порядке вызова spawn.
        void spawn(std::function<void()> body);
        // Запустить потоки и вести их по одному: на каждой паузе спросить strategy.pick(enabled, last).
        void run(Strategy &strategy);
        // Трасса прогона: буквы потоков, выбранных во всех решениях подряд, например "AAAABB".
        std::string trace_string() const;

        // Если true, каждая операция с общими данными добавляет строку в log. Выставляет runner.
        bool logging = false;
        // Лог прогона: строки "<поток>  <операция>", например "A  lock mu", "A  balance -> 100".
        std::vector<std::string> log;
        // Пустая строка, если deadlock не было; иначе "held={a:A, b:B}" (этап 2).
        std::string deadlock;

        void wait_for_lock(sched_mutex *m);
        void unlock(sched_mutex *m);

        void register_lock(sched_mutex *m);

    private:
        friend void point();
        friend void await(std::function<bool()> cond);
        friend void before_shared_access();
        friend void record_operation(const std::string &operation);

        void before_operation(Thread &worker);
        void pause(Thread &worker, ThreadState state);
        void join_workers();
        void abort_and_join();

        std::vector<std::unique_ptr<Thread>> threads;
        std::binary_semaphore yielded{0};
        std::string trace;
        bool started = false;
        bool aborted = false;
        std::vector<sched_mutex *> locks;
    };

    // window() из тестовых программ: пауза перед следующей операцией с общими данными обязательна.
    void point();
    // Ждать, пока cond не станет true. До этого поток не попадает в enabled.
    void await(std::function<bool()> cond);

    // Instrumentation hooks used by shared<T> (and, later, sched_mutex).
    void before_shared_access();
    void record_operation(const std::string &operation);
} // namespace sched
