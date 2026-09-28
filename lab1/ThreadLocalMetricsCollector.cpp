#include "ThreadLocalMetricsCollector.hpp"
#include "util.hpp"
#include <atomic>
#include <cstdint>
#include <mutex>
inline void relaxed_add(std::atomic<uint64_t> &c, uint64_t delta) {
  c.store(c.load(std::memory_order_relaxed) + delta, std::memory_order_relaxed);
}

void ThreadLocalMetricsCollector::record(uint64_t value) {
  ThreadState *s = get_my_state();
  uint64_t b = std::min(value / 4, (uint64_t)255);

  relaxed_add(s->buckets[b], 1);
  relaxed_add(s->count, 1);
  relaxed_add(s->sum, value);

  if (value < s->min.load(std::memory_order_relaxed))
    s->min.store(value, std::memory_order_relaxed);
  if (value > s->max.load(std::memory_order_relaxed))
    s->max.store(value, std::memory_order_relaxed);
}

Snapshot ThreadLocalMetricsCollector::snapshot() {
  std::array<uint64_t, 256> buckets{};
  uint64_t count = 0, sum = 0, min = UINT64_MAX, max = 0;
  list_lock_.lock();
  std::vector<ThreadState *> _all_states;
  _all_states.reserve(_all_states.size());
  for (const auto &p : all_states_) {
    _all_states.push_back(p.get());
  }
  list_lock_.unlock();

  for (auto *s : _all_states) {
    for (int i = 0; i < 256; i++) {
      buckets[i] += s->buckets[i].load(std::memory_order_relaxed);
    }
    count += s->count.load(std::memory_order_relaxed);
    sum += s->sum.load(std::memory_order_relaxed);
    min = std::min(min, s->min.load(std::memory_order_relaxed));
    max = std::max(max, s->max.load(std::memory_order_relaxed));
  }

  return Snapshot{buckets,
                  count,
                  sum,
                  min,
                  max,
                  get_percentile(0.5, buckets, count),
                  get_percentile(0.99, buckets, count)};
}
