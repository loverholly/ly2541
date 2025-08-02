#!/bin/bash
#todo: add the cross_compiler for arm
make -j32 CROSS_COMPILE=arm-linux-gnueabihf- TEST_UNIT=1 BUILD_TYPE=DEBUG
scp bin/mainApp root@192.168.3.11:~/
