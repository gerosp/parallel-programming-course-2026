#pragma once
#include "MetricsCollector.hpp"
#include <atomic>
#include <mutex>
class ShardedMetricsCollector final : public MetricsCollector {
public:
  virtual void record(uint64_t value) override;
  virtual Snapshot snapshot() override;

private:
  std::atomic<uint64_t> _sum{0};
  std::atomic<uint64_t> _count{0};
  std::atomic<uint64_t> _max{0};
  std::atomic<uint64_t> _min{UINT64_MAX};
  std::array<std::mutex,16> m_array{};
};
