#!/bin/bash

source tools/kill_qemu.sh
source tools/mkflash.sh
source tools/restart-net-servers.sh


sudo ./qemu-system-arm -M mini2440 \
				-serial stdio \
				-mtdblock image/nand.bin \
				-kernel image/uImage \
				-usb \
				-usbdevice mouse \
				-usbdevice keyboard \
                -usbdevice tablet \
                -show-cursor \
				-net nic,vlan=0 \
                -net tap,vlan=0,ifname=tap0,script=./tools/qemu-ifup,downscript=./tools/qemu-ifdown \
                -S -s &

sleep 2
#xfce4-terminal -e "ddd --debugger arm-linux-gnueabi-gdb -x tools/gdbinit"
xfce4-terminal -e "ddd --debugger /opt/gcc-linaro-4.9-2016.02-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-gdb -x tools/gdbinit"


