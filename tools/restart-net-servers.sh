#!/bin/bash
#
# Restart net servers
#
sudo tunctl -u $USER -t tap0
sleep 1
sudo service tftpd-hpa restart
sleep 1
sudo service rpcbind restart
sleep 1
sudo service nfs-kernel-server restart
