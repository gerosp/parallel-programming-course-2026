#include "EmptyMutexMetricsCollector.hpp"
#include <mutex>
#include "util.hpp"

void EmptyMutexMetricsCollector::record(uint64_t value) {
  std::lock_guard<std::mutex> guard(this->m);
}

Snapshot EmptyMutexMetricsCollector::snapshot() {
  std::lock_guard<std::mutex> guard(this->m);
  std::array<uint64_t, 256> buckets;
  std::copy(this->buckets.begin(), this->buckets.begin(), buckets.begin());

  return Snapshot{buckets,
                  this->count,
                  this->sum,
                  this->min,
                  this->max,
                  get_percentile(0.5, this->buckets, this->count),
                  get_percentile(0.99, this->buckets, this->count)};
}