1. need add the cross-compiler setting for arm.
2. when you add the new module, please add the gtest unit test.
3. ./buildtest.sh should ensure the test/x86_64-linux-gnu.cmake obsolute path is ~/project/ly2541/test/x86_64-linux-gnu.cmake

** static ip config
1. if no backup
vi /etc/network/interface
auto eth0
iface eth0 static
	ipaddr  192.168.0.10
	netmask 255.255.255.0
2. if has backup file
vi /etc/interface.bak
auto eth0
iface eth0 static
	ipaddr  192.168.0.10
	netmask 255.255.255.0

** config app autorun
1. touch /etc/profile.d/apprun.sh
2. vi /etc/profile.d/apprun.sh
   #!/bin/bash
   cd /home/root/ && ./mainApp &
