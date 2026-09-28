#include "MetricsCollector.hpp"
#include "util.hpp"
#include <cstdint>

void MetricsCollector::record(uint64_t value) {};


Snapshot MetricsCollector::snapshot() {
  std::array<uint64_t, 256> buckets;
  std::copy(this->buckets.begin(), this->buckets.begin(), buckets.begin());

  return Snapshot{buckets,
                        this->count,
                        this->sum,
                        this->min,
                        this->max,
                        get_percentile(0.5, this->buckets, this->count),
                        get_percentile(0.99, this->buckets, this->count)};
};