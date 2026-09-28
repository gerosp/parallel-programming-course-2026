#pragma once

#include "MetricsCollector.hpp"
#include <mutex>

class MutexMetricsCollector final : public MetricsCollector {
public:
    virtual void record(uint64_t value) override;
    virtual Snapshot snapshot() override;
private:
    std::mutex m;

};
