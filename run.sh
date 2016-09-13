#./qemu-system-arm -M mini2440 -nographic -serial stdio -mtdblock image/nand.bin -usbdevice mouse
#./configure --target-list=arm-softmmu --disable-vnc-tls --disable-curses

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
                -net tap,vlan=0,ifname=tap0,script=./tools/qemu-ifup,downscript=./tools/qemu-ifdown

##-monitor telnet::5555,server,nowait
##-pflash image/nor.bin
exit
sudo ./qemu-system-arm -M mini2440 -nographic -serial stdio -mtdblock image/nand.bin

