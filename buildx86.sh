#!/bin/bash
#make -j32 BUILD_TYPE=RELEASE
#make -f build/Makefile clean
echo "正在构建 X86 版本"
make -f build/Makefile -j32 BUILD_TYPE=DEBUG CC=gcc CXX=g++
