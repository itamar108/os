
cmake_minimum_required(VERSION 3.12.2)  # CMake version check
#cmake_minimum_required(VERSION 3.13)  # CMake version check


project(../os_ex1)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
# Add main.cpp file of project root directory as source file
set(SOURCE_FILES ../main.cpp)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(ex1 ../osm.cpp ../osm.h ../main.cpp ../../ex2/src/uthreads.cpp)