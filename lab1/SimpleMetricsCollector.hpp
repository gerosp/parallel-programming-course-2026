#pragma once

#include "MetricsCollector.hpp"


class SimpleMetricsCollector final : public MetricsCollector {
public:
    virtual void record(uint64_t value) override;

};
