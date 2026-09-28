#pragma once

#include "MetricsCollector.hpp"
#include "DoubleBufferedMetricsCollector.hpp"
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>
#include <memory>

inline uint64_t db_no_check_next_collector_id() {
  static std::atomic<uint64_t> counter{1};
  return counter.fetch_add(1);
}




class DoubleBufferedNoActiveCheckMetricsCollector final : public MetricsCollector {

  const uint64_t id_ = db_no_check_next_collector_id();
  std::vector<std::unique_ptr<ThreadBuffers>> all_states_;
  std::atomic<size_t> active = 0;
  std::mutex snap_lock_;

  ThreadBuffers *get_my_state() {
    struct TLSSlot {
      uint64_t id = 0;
      ThreadBuffers *state = nullptr;
    };
    static thread_local TLSSlot slot;
    if (slot.id != id_) {
      auto s = std::make_unique<ThreadBuffers>();
      ThreadBuffers *raw = s.get();
      {
        std::lock_guard<std::mutex> g(snap_lock_);
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
