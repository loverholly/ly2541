#!/bin/bash
cd test && rm -rf build
mkdir build && cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=~/project/ly2541/test/x86_64-linux-gnu.cmake && make -j32 CC=gcc CXX=g++ && ctest && lcov --capture --directory . --output-file coverage.info && lcov --list coverage.info && genhtml coverage.info --output-directory coverage_report --title "My Project Coverage"

