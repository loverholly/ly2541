# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Enable testing
enable_testing()

# gtest
include_directories(/usr/local/include)
include_directories(../include)
link_directories(/usr/local/lib)

# Add the add function
add_library(add ../src/fpga_ctrl.c)
target_include_directories(add PUBLIC src)

# Add the test
add_executable(test_add ../test/test_fpga_ctrl.cpp)
target_link_libraries(test_add gtest gtest_main add pthread)

# Add a test
add_test(NAME AddTest COMMAND test_add)

# Enable code coverage
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
target_link_libraries(add gcov)
