#!/bin/bash



cd u-boot
rm -rf ../.git/modules/u-boot/index.lock
git reset --hard aeff6d50
git clean -d -fx
cd ..

cd linux-stable
rm -rf ../.git/modules/linux-stable/index.lock
git checkout -b linux-2.6.32.y
git branch --set-upstream-to=origin/linux-2.6.32.y linux-2.6.32.y
git reset --hard v2.6.32.27
git clean -d -fx
cd ..