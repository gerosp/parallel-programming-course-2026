#include "ShardedMetricsCollector.hpp"
#include "util.hpp"
#include <cstdint>
#include <mutex>

void ShardedMetricsCollector::record(uint64_t value) {
  size_t b = std::min(value / 4ULL, 255ULL);
  this->m_array[b % 16].lock();
  buckets[b]++;
  this->m_array[b % 16].unlock();
  this->_count++;
  this->_sum += value;
  uint64_t old = _min.load();
  if (old > value)
    while (old > value && !_min.compare_exchange_weak(old, value)) {
    };
  old = _max.load();
  if (old < value)
    while (old < value && !_max.compare_exchange_weak(old, value)) {
    };
}

Snapshot ShardedMetricsCollector::snapshot() {
  std::array<uint64_t, 256> buckets;
  for (int bucket = 0; bucket < 16; bucket++) {
    this->m_array[bucket].lock();
    for (int i = bucket; i < buckets.size(); i += 16) {
      buckets[i] = this->buckets[i];
    }
    this->m_array[bucket].unlock();
  }
  //   std::copy(this->buckets.begin(), this->buckets.begin(), buckets.begin());
  uint64_t count = _count.load();
  return Snapshot{buckets,
                  count,
                  this->_sum.load(),
                  this->_min.load(),
                  this->_max.load(),
                  get_percentile(0.5, buckets, count),
                  get_percentile(0.99, buckets, count)};
}