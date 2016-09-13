#!/bin/bash



cd u-boot
rm -rf ../.git/modules/u-boot/index.lock
git pull
cd ..

cd linux-stable
rm -rf ../.git/modules/linux-stable/index.lock
git checkout -b linux-3.4.y
git branch --set-upstream-to=origin/linux-3.4.y linux-3.4.y
git pull
cd ..