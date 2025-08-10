#!/bin/bash
#make -f build/Makefile clean
echo "正在构建 Xilinx 版本"
source /opt/petalinux2021.2/environment-setup-cortexa72-cortexa53-xilinx-linux
make -f build/Makefile -j32 CROSS_COMPILE=aarch64-xilinx-linux- BUILD_TYPE=DEBUG \
	EXTRA_CFLAGS="-mcpu=cortex-a72.cortex-a53 \
	-march=armv8-a+crc -fstack-protector-strong \
	-D_FORTIFY_SOURCE=2 -Wformat -Wformat-security \
	-Werror=format-security \
	--sysroot=/opt/petalinux2021.2/sysroots/cortexa72-cortexa53-xilinx-linux"
scp bin/mainApp root@192.168.60.11:~/
