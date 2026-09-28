#pragma once
#include <array>
#include <vector>
#include <cstdint>

double get_median(std::vector<double> vec);
uint64_t get_percentile(double percentile, std::array<uint64_t, 256> buckets,
                        uint64_t count);