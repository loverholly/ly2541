#!/bin/bash
cd test && rm -rf build
mkdir build && cd build && cmake .. && make -j32 && ctest && lcov --capture --directory . --output-file coverage.info && lcov --list coverage.info && genhtml coverage.info --output-directory coverage_report --title "My Project Coverage"

