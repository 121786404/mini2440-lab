#!/bin/bash



echo "========begin build u-boot========="

source ./build-uboot.sh >/dev/null 2>&1

echo "========begin build linux========="

source ./build-linux.sh >/dev/null 2>&1
