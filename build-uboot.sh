#!/bin/bash


cd u-boot
git reset --hard HEAD
git pull
git clean -f
cp ../tools/u-boot-patch/* . -r
export ARCH=arm
export CROSS_COMPILE=arm-linux-
make mini2440_config
make
arm-linux-gnueabi-objdump -D -m arm u-boot > u-boot.dis
cp u-boot.bin ../image
cd ..