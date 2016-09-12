#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# FIC HXD8 board with S3C2440X (ARM920T) cpu
#
# see http://www.samsung.com/ for more information on SAMSUNG
#

CONFIG_USB_DFU_VENDOR=0x1457
CONFIG_USB_DFU_PRODUCT=0x511a
CONFIG_USB_DFU_REVISION=0x0110

#
# HXD81v011 or later has 1 bank of 128 MB SDRAM
#
# 	3000'0000 to 3800'0000
# we load ourself to 37F8'0000
#
# Linux-Kernel is expected to be at 3000'8000, entry 3000'8000
# optionally with a ramdisk at 3080'0000
#
# download area is 3200'0000 or 3300'0000

# FIXME: TEXT_BASE = 0x37F80000
TEXT_BASE = 0x33F80000
