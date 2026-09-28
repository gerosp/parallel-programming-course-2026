#include "DoubleBufferedMetricsCollector.hpp"
#include "util.hpp"
#include <atomic>
#include <thread>
void DoubleBufferedMetricsCollector::record(uint64_t value) {
  ThreadBuffers *my = get_my_state();
  int b;
  while (true) {
    b = active.load(std::memory_order_seq_cst);
    my->inside.store(b, std::memory_order_seq_cst);
    if (active.load(std::memory_order_seq_cst) == b) {
      break;
    }
    my->inside.store(-1, std::memory_order::seq_cst);
  }
  auto bucket = std::min(value / 4ULL, 255ULL);
  my->buf[b].buckets[bucket]++;
  my->buf[b].count++;
  my->buf[b].sum += value;
  my->buf[b].max = std::max(my->buf[b].max, value);
  my->buf[b].min = std::min(my->buf[b].min, value);
  my->inside.store(-1, std::memory_order_seq_cst);
}

Snapshot DoubleBufferedMetricsCollector::snapshot() {

  std::lock_guard<std::mutex> g(snap_lock_);
  auto old = active.load(std::memory_order::seq_cst);
  active.store(1 - old, std::memory_order_seq_cst);

  for (auto &s : all_states_) {
    while (s->inside.load() == old) {
      std::this_thread::yield();
    }
    this->count += s->buf[old].count;
    this->sum += s->buf[old].sum;
    this->max = std::max(s->buf[old].max, max);
    this->min = std::min(s->buf[old].min, min);
    for (int i = 0; i < 256; i++) {
      buckets[i] += s->buf[old].buckets[i];
    }
    s->buf[old].clear();
  }

  return Snapshot{buckets,
                  count,
                  sum,
                  min,
                  max,
                  get_percentile(0.5, buckets, count),
                  get_percentile(0.99, buckets, count)};
}