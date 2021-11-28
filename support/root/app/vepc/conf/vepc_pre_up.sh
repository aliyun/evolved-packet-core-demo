#!/bin/bash

# S1 ip
#ip addr add dev enp3s0 192.168.202.1/30 >/dev/null 2>&1

#arping -c 5 -U -i enp3s0 192.168.202.1 >/dev/null 2>&1 &
#arping -c 5 -i enp3s0 -s 192.168.202.1 192.168.202.2 >/dev/null 2>&1 &

# Firewall
iptables -D INPUT -i enp3s0 -j ACCEPT >/dev/null 2>&1
iptables -I INPUT -i enp3s0 -j ACCEPT >/dev/null 2>&1
iptables -D INPUT -i enp7s0f1 -j ACCEPT >/dev/null 2>&1
iptables -I INPUT -i enp7s0f1 -j ACCEPT >/dev/null 2>&1

# NAT
iptables -t nat -D POSTROUTING -o enp2s0 -s 41.1.0.0/16 -j MASQUERADE >/dev/null 2>&1
iptables -t nat -I POSTROUTING 1 -o enp2s0 -s 41.1.0.0/16 -j MASQUERADE >/dev/null 2>&1

iptables -D FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1420
iptables -I FORWARD 1 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1420
