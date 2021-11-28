#!/bin/bash
# chkconfig: 2345 10 90 
# description: vepc_pre_init

#ip tuntap add name pgwtun mode tun
#ip addr add 41.1.0.1/16 dev pgwtun
#ip addr add cafe::1/64 dev pgwtun
#ip link set pgwtun up
INSTALL_DIR=/home/wnet/.epc/install
CONFIG_DIR=/home/wnet/.epc/config

killall -9 vepc-epcd
rmmod gtp_fw
insmod $INSTALL_DIR/kmod/gtp_fw.ko
$INSTALL_DIR/bin/vepc-epcd -f $CONFIG_DIR/vepc.conf -l $INSTALL_DIR/var/log/vepc/vepc.log -d
