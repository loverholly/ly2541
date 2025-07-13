#!/bin/bash
#todo: add the cross_compiler for arm
make -j32 CROSS_COMPILE=aarch64-linux-gnu- TEST_UNIT=1 BUILD_TYPE=RELEASE
scp bin/mainApp root@192.168.3.11:~/
