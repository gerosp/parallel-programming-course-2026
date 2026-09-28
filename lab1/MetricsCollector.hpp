#pragma once
#include <array>
#include <cstdint>
#include <limits>
struct Snapshot {
  std::array<uint64_t, 256> buckets;
  uint64_t count;
  uint64_t sum;
  uint64_t min;
  uint64_t max;
  uint64_t p50;
  uint64_t p99;
};

class MetricsCollector {
public:
  virtual ~MetricsCollector() = default;
  virtual void record(uint64_t value);
  virtual Snapshot snapshot();

protected:
  uint64_t count = 0;
  uint64_t sum = 0;
  uint64_t max = 0;
  uint64_t min = std::numeric_limits<uint64_t>::max();
  std::array<uint64_t, 256> buckets{};
};
