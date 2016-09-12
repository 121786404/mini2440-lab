git clone --recursive https://github.com/121786404/mini2440-lab

sudo apt-get install libsdl1.2debian:i386 uml-utilities mtd-utils ddd

linux-3.7和linux-3.8-rc1之间的修改导致
at drivers/mtd/nand/nand_base.c:867 nand_wait+0x170/0x190()  进而crash