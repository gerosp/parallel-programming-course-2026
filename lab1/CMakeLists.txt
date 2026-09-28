add_library(lab1_core
    DataGenerator.cpp
    MetricsCollector.cpp
    SimpleMetricsCollector.cpp
    ShardedMetricsCollector.cpp
    MutexMetricsCollector.cpp
    EmptyMutexMetricsCollector.cpp
    ThreadLocalMetricsCollector.cpp
    DoubleBufferedMetricsCollector.cpp
    DoubleBufferedMetricsCollectorNoActiveCheck.cpp
    BenchRunner.cpp
    util.cpp
)
target_include_directories(lab1_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lab1_core PUBLIC Threads::Threads)

add_executable(lab1_bench main.cpp)
target_link_libraries(lab1_bench PRIVATE lab1_core)

if(LAB1_ENABLE_TSAN)
    add_custom_target(lab1_tsan_run
        COMMAND $<TARGET_FILE:lab1_bench>
        DEPENDS lab1_bench
        USES_TERMINAL
    )
endif()
