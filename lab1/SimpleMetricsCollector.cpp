#include "SimpleMetricsCollector.hpp"
#include <algorithm>

void SimpleMetricsCollector::record(uint64_t value) {
    this->sum += value;
    this->count++;
    if (this->min > value)
      this->min = value;
    if (this->max < value)
      this->max = value;
    buckets[std::min(value / 4ULL, 255ULL)]++;
}
