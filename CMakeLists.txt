cmake_minimum_required(VERSION 3.22)
project(dataframe_cpp LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use C++23 everywhere
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Debug unless the user picks another config
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_executable(dataframe src/main.cpp)

# Warnings & debug flags for GCC/Clang
target_compile_options(dataframe PRIVATE -Wall -Wextra -Wpedantic -O0 -g)

# Optional: sanitizers for easier debugging (uncomment to enable)
# target_compile_options(hello PRIVATE -fsanitize=address,undefined)
# target_link_options(hello PRIVATE -fsanitize=address,undefined)