#pragma once
#include "MetricsCollector.hpp"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>
#include <memory>
inline uint64_t next_collector_id() {
  static std::atomic<uint64_t> counter{1};
  return counter.fetch_add(1);
}
struct alignas(64) ThreadState {
  std::array<std::atomic<uint64_t>, 256> buckets{};
  std::atomic<uint64_t> count{0};
  std::atomic<uint64_t> sum{0};
  std::atomic<uint64_t> min{UINT64_MAX};
  std::atomic<uint64_t> max{0};
};

class ThreadLocalMetricsCollector final : public MetricsCollector {

  const uint64_t id_ = next_collector_id();
  std::mutex list_lock_;
  std::vector<std::unique_ptr<ThreadState>> all_states_;

  ThreadState *get_my_state() {
    struct TLSSlot {
      uint64_t id = 0;
      ThreadState *state = nullptr;
    };
    static thread_local TLSSlot slot;
    if (slot.id != id_) {
      auto s = std::make_unique<ThreadState>();
      ThreadState *raw = s.get();
      {
        std::lock_guard<std::mutex> g(list_lock_);
        all_states_.push_back(std::move(s));
      }
      slot.id = id_;
      slot.state = raw;
    }
    return slot.state;
  }

public:
  virtual void record(uint64_t value) override;
  virtual Snapshot snapshot() override;
};
