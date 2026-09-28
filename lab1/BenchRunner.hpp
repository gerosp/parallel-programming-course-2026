#pragma once
#include "MetricsCollector.hpp"
#include <cstddef>
#include <cstdint>

double run(MetricsCollector& collector, const std::array<uint64_t, 1048576>& values, size_t T,
         uint64_t seconds);

double measure_point(MetricsCollector& collector, const std::array<uint64_t, 1048576>& values, uint64_t T);


void inconsistency_test(MetricsCollector& collector, const std::array<uint64_t, 1048576>& values, uint64_t T);