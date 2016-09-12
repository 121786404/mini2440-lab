#!/bin/bash


cd image
sudo rm -rf rootfs
mkdir -p rootfs
sudo tar xfap ../tools/rootfs.tar -C rootfs
sudo cp ../tools/mount_nfs.sh rootfs/root
sudo mkfs.jffs2 -r rootfs -o rootfs.jffs2 -e 16KiB -s 0x200 -n
sudo rm -rf rootfs
rm -rf nand.bin

../tools/flashimg -s 64M -t nand -f nand.bin -p ../tools/uboot.part -w boot,u-boot.bin -w kernel,uImage -w root,rootfs.jffs2 -z 512
#../tools/flashimg -s 2M -t nor -f nor.bin -p uboot_nor.part -w boot,u-boot.bin
cd ..
