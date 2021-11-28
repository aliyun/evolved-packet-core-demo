#!/bin/bash

ethtool -K enp2s0 gro off
ethtool -K enp3s0 gro off
ethtool -K enp6s0f0 gro off

ifconfig enp2s0 100.100.100.2/30 up
ifconfig enp3s0 100.100.100.6/30 up
ifconfig enp6s0f0 192.168.27.10/24 up

iptables -t nat -F POSTROUTING
iptables -t nat -A POSTROUTING -o enp6s0f0 -s 10.10.0.0/16 -j MASQUERADE
iptables -F FORWARD
iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1420

route add default gw 192.168.27.254 dev enp6s0f0
route add -net 7.99.142.0/26 gw 100.100.100.1 dev enp2s0
route add -net 7.99.141.128/26 gw 100.100.100.5 dev enp3s0

#rmmod gtp_fw
#insmod /epc/kmod/gtp_fw.ko
/epc/bin/vepc-epcd -d

killall -9 epc_check.sh

/epc/bin/epc_check.sh &

sleep 5
ping 100.100.100.1 -c 10 &
ping 100.100.100.5 -c 10 &

ipsec up wnet_vpn &
