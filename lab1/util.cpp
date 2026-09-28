#include "util.hpp"
#include <algorithm>

double get_median(std::vector<double> vec) {
  if (vec.empty())
    return 0.0;

  size_t n = vec.size();
  auto mid_it = vec.begin() + n / 2;

  std::nth_element(vec.begin(), mid_it, vec.end());

  if (n % 2 != 0) {
    return *mid_it;
  } else {
    auto left_mid_it = std::max_element(vec.begin(), mid_it);
    return (*left_mid_it + *mid_it) / 2.0;
  }
}

uint64_t get_percentile(double percentile, std::array<uint64_t, 256> buckets,
                        uint64_t count) {
  const double threshold = count * percentile;
  double sum = 0;
  for (int i = 0; i < 256; i++) {
    sum += buckets[i];
    if (sum >= threshold)
      return i * 4;
  }
  return 0;
}