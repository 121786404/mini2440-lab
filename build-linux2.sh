#!/bin/bash

rm -rf image/uImage
cd linux-stable
git clean -d -fx
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabi-
cp ../tools/my_mini2440_defconfig arch/arm/configs
make my_mini2440_defconfig
make zImage -j4
make uImage
cp arch/arm/boot/uImage ../image
../tools/collect-src -o ../linux_src.list
cd ..
