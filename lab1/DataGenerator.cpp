#include "DataGenerator.hpp"
#include <algorithm>
#include <cmath>
void generate_data(std::array<uint64_t, 1048576> &data) {
  srand(0);
  std::array<double, 1024> cdf {0};
  double prob = 0.0;
  for (size_t k = 1; k <= 1023; ++k) {
    prob += 1.0 / std::pow(static_cast<double>(k), 1.15);
    cdf[k] = prob;
  }
  for (auto &c : cdf) {
    c /= prob;
  }
  for (auto &value : data) {
    const double urnd =
        static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
    auto zrnd = std::lower_bound(cdf.begin(), cdf.end(), urnd);
    value = static_cast<uint64_t>(abs(cdf.begin() - zrnd));
  }
}
