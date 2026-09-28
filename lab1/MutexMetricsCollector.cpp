#include "MutexMetricsCollector.hpp"
#include "util.hpp"
#include <mutex>

void MutexMetricsCollector::record(uint64_t value) {
  std::lock_guard<std::mutex> guard(this->m);
  this->sum += value;
  this->count++;
  if (this->min > value)
    this->min = value;
  if (this->max < value)
    this->max = value;
  buckets[std::min(value / 4ULL, 255ULL)]++;
}

Snapshot MutexMetricsCollector::snapshot() {
  std::lock_guard<std::mutex> guard(this->m);
  std::array<uint64_t, 256> buckets;
  std::copy(this->buckets.begin(), this->buckets.begin(), buckets.begin());

  return Snapshot{
      buckets, this->count, this->sum, this->min, this->max,
      get_percentile(0.5, this->buckets,
                     this->count), // this->buckets is NOT synchronised if the
                                   // whole function is not locked
      get_percentile(0.99, this->buckets, this->count)};
}