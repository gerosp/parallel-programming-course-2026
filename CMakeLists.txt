cmake_minimum_required(VERSION 3.20)
project(parallel_programming_course LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LAB1_ENABLE_TSAN "Build lab1 with ThreadSanitizer (Linux GCC/Clang)" OFF)
if(LAB1_ENABLE_TSAN)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
       NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "LAB1_ENABLE_TSAN requires Linux with GCC or Clang")
    endif()
    add_compile_options(-fsanitize=thread -g -O1 -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()

find_package(Threads REQUIRED)
add_subdirectory(lab1)
