#include "BenchRunner.hpp"
#include "MetricsCollector.hpp"
#include "util.hpp"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <latch>
#include <numeric>
#include <thread>
#include <vector>

void thread_task(size_t k, std::latch &start, std::atomic<bool> &stop,
                 MetricsCollector &collector,
                 const std::array<uint64_t, 1048576> &values, uint64_t ops[]) {
  uint64_t local_count = 0;
  size_t i = k * 1000;
  start.wait();
  while (!stop) {
    collector.record(values[i]);
    local_count++;
    i++;
    if (i == values.size())
      i = 0;
  }
  ops[k] = local_count;
}

double run(MetricsCollector &collector,
           const std::array<uint64_t, 1048576> &values, size_t T,
           uint64_t seconds) {
  std::latch start{1};
  std::atomic<bool> stop = false;
  std::vector<uint64_t> ops(T);
  std::vector<std::thread> threads(T);
  for (size_t i = 0; i < T; i++) {
    threads[i] = std::thread([&, i] {
      thread_task(i, start, stop, collector, values, ops.data());
    });
  }
  auto t0 = std::chrono::steady_clock::now();
  start.count_down();
  std::this_thread::sleep_for(std::chrono::seconds(seconds));
  stop = true;
  auto t1 = std::chrono::steady_clock::now();
  for (size_t i = 0; i < T; i++) {
    threads[i].join();
  }
  const double elapsed = std::chrono::duration<double>(t1 - t0).count();
  return static_cast<double>(std::accumulate(ops.begin(), ops.end(), 0ULL)) /
         elapsed;
}

double measure_point(MetricsCollector &collector,
                     const std::array<uint64_t, 1048576> &values, uint64_t T) {
  run(collector, values, T, 5);
  std::vector<double> results;
  for (int i = 0; i < 5; i++) {
    results.push_back(run(collector, values, T, 5));
  }
  std::cout << collector.snapshot().count << std::endl;
  return get_median(results);
}

void inconsistency_test(MetricsCollector &collector,
                        const std::array<uint64_t, 1048576> &values,
                        uint64_t T) {
  std::latch start{1};
  std::atomic<bool> stop = false;
  std::vector<uint64_t> ops(T);
  std::vector<std::thread> threads(T);
  for (size_t i = 0; i < T; i++) {
    threads[i] = std::thread([&, i] {
      thread_task(i, start, stop, collector, values, ops.data());
    });
  }
  start.count_down();
  // std::this_thread::sleep_for(std::chrono::seconds(5));
  int overshoots = 0, undershoots = 0, broken = 0;
  for (size_t i = 0; i < 10000; i++) {
    Snapshot snapshot = collector.snapshot();
    uint64_t sum =
        std::accumulate(snapshot.buckets.begin(), snapshot.buckets.end(), 0ULL);
    if (sum > snapshot.count) {
      undershoots += 1;
      broken++;
    } else if (sum < snapshot.count) {
      overshoots += 1;
      broken++;
    }
  }
  stop = true;
  for (size_t i = 0; i < T; i++) {
    threads[i].join();
  }
  Snapshot snapshot = collector.snapshot();
  const uint64_t local_count = std::accumulate(ops.begin(), ops.end(), 0ULL);
  std::cout << "Consistency test:\nBroken: " << broken
            << "\nBroken(%): " << static_cast<double>(broken) / 10000.0 * 100.0
            << "%\nOvershoots: " << overshoots
            << "\nUndershoots: " << undershoots
            << "\nCount(snapshot): " << snapshot.count
            << "\nCount(ops): " << local_count << std::endl;
}