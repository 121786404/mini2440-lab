#!/bin/bash

rm -rf image/u-boot.bin
cd u-boot
git reset --hard aeff6d50
git clean -f
cp ../tools/u-boot-patch-aeff6d50/* . -r
export ARCH=arm
export CROSS_COMPILE=arm-linux-
make mini2440_config
make
arm-linux-gnueabi-objdump -D -m arm u-boot > u-boot.dis
cp u-boot.bin ../image
cd ..