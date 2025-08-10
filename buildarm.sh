#!/bin/bash
#make -f build/Makefile clean
echo "正在构建 ARM 版本"
make -f build/Makefile -j32 CROSS_COMPILE=arm-linux-gnueabihf- TEST_UNIT=1 BUILD_TYPE=DEBUG
scp bin/mainApp root@192.168.3.11:~/
