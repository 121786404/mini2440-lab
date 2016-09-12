#!/bin/bash


cd u-boot
cp ../tools/u-boot-patch-aeff6d50/* . -r
export ARCH=arm
export CROSS_COMPILE=arm-linux-
make mini2440_config
make
cp u-boot.bin ../image
cd ..