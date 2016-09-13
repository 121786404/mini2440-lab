#!/bin/bash

rm -rf image/u-boot.bin
cd u-boot
git reset --hard HEAD
git clean -d -fx
git pull
cp ../tools/u-boot-patch/* . -r
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabi-
make mini2440_config
make
arm-linux-gnueabi-objdump -D -m arm u-boot > u-boot.dis
cp u-boot.bin ../image
cd ..