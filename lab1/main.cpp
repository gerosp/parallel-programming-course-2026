#include "BenchRunner.hpp"
#include "DataGenerator.hpp"
#include "DoubleBufferedMetricsCollector.hpp"
#include "DoubleBufferedMetricsCollectorNoActiveCheck.hpp"
#include "EmptyMutexMetricsCollector.hpp"
#include "MutexMetricsCollector.hpp"
#include "ShardedMetricsCollector.hpp"
#include "SimpleMetricsCollector.hpp"
#include "ThreadLocalMetricsCollector.hpp"
#include <iostream>
#include <memory>
void measure_P0_single_thread(const std::array<uint64_t, 1048576> &data) {
  SimpleMetricsCollector collector;
  std::cout << "Single thread result: " << measure_point(collector, data, 1)
            << std::endl;
}
void measure_P1_mutex(const std::array<uint64_t, 1048576> &data, size_t T) {
  MutexMetricsCollector collector;
  std::cout << "Mutex result (" << T
            << "): " << measure_point(collector, data, T) << std::endl;
}
void measure_P1_empty_mutex(const std::array<uint64_t, 1048576> &data,
                            size_t T) {
  EmptyMutexMetricsCollector collector;
  std::cout << "Empty Mutex result (" << T
            << "): " << measure_point(collector, data, T) << std::endl;
}
void measure_P2_sharded_mutex(const std::array<uint64_t, 1048576> &data,
                              size_t T) {
  ShardedMetricsCollector collector;
  std::cout << "Sharded Mutex result (" << T
            << "): " << measure_point(collector, data, T) << std::endl;
}
void inconsistency_P2_sharded_mutex(const std::array<uint64_t, 1048576> &data,
                                    size_t T) {
  ShardedMetricsCollector collector;
  std::cout << "P2 Shared Mutex consistency check" << std::endl;
  inconsistency_test(collector, data, T);
}
void measure_P3_thread_local(const std::array<uint64_t, 1048576> &data,
                             size_t T) {
  ThreadLocalMetricsCollector collector;
  std::cout << "Thread Local result (" << T
            << "): " << measure_point(collector, data, T) << std::endl;
}

void inconsistency_P3_thread_local(const std::array<uint64_t, 1048576> &data,
                                   size_t T) {
  ThreadLocalMetricsCollector collector;
  std::cout << "P3 Thread Local consistency check" << std::endl;

  inconsistency_test(collector, data, T);
}

void measure_P4_double_buffer(const std::array<uint64_t, 1048576> &data,
                              size_t T) {
  DoubleBufferedMetricsCollector collector;
  std::cout << "Double buffer result (" << T
            << "): " << measure_point(collector, data, T) << std::endl;
}

void inconsistency_P4_double_buffer(const std::array<uint64_t, 1048576> &data,
                                    size_t T) {
  DoubleBufferedMetricsCollector collector;
  std::cout << "P4 Double Buffer consistency check" << std::endl;
  inconsistency_test(collector, data, T);
}
void inconsistency_P4_double_buffer_no_active_check(
    const std::array<uint64_t, 1048576> &data, size_t T) {
  DoubleBufferedNoActiveCheckMetricsCollector collector;
  std::cout << "P4 Double Buffer consistency check (NO SECOND ACTIVE CHECK)"
            << std::endl;
  inconsistency_test(collector, data, T);
}
int main() {
  auto data = std::make_unique<std::array<uint64_t, 1048576>>();
  generate_data(*data);
  measure_P0_single_thread(*data);
  for (size_t T = 1; T <= 16; T *= 2)
    measure_P1_mutex(*data, T);
  for (size_t T = 1; T <= 16; T *= 2)
    measure_P1_empty_mutex(*data, T);
  for (size_t T = 1; T <= 16; T *= 2)
    measure_P2_sharded_mutex(*data, T);

  inconsistency_P2_sharded_mutex(*data, 4);

  for (size_t T = 1; T <= 16; T *= 2)
    measure_P3_thread_local(*data, T);

  inconsistency_P3_thread_local(*data, 4);

  for (size_t T = 1; T <= 16; T *= 2)
    measure_P4_double_buffer(*data, T);

  inconsistency_P4_double_buffer(*data, 4);

  inconsistency_P4_double_buffer_no_active_check(*data, 4);
}
