#!/bin/bash

#ifconfig enp2s0 100.100.100.2/30 up
#ifconfig enp3s0 100.100.100.6/30 up
#ifconfig enp6s0f0 192.168.27.10/24 up

#iptables -t nat -F POSTROUTING
#iptables -t nat -A POSTROUTING -o enp6s0f0 -s 10.10.0.0/16 -j MASQUERADE
#iptables -F FORWARD
#iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1420

#route add default gw 192.168.27.254 dev enp6s0f0
#route add -net 7.99.142.0/26 gw 100.100.100.1 dev enp2s0
#route add -net 7.99.141.128/26 gw 100.100.100.5 dev enp3s0

# S1 ip
ip addr add dev enp3s0 192.168.202.1 >/dev/null 2>&1

# Firewall
iptables -D INPUT -i enp3s0 -j ACCEPT >/dev/null 2>&1
iptables -I INPUT -i enp3s0 -j ACCEPT >/dev/null 2>&1
iptables -D INPUT -i enp7s0f1 -j ACCEPT >/dev/null 2>&1
iptables -I INPUT -i enp7s0f1 -j ACCEPT >/dev/null 2>&1

# NAT
iptables -t nat -D POSTROUTING -o enp2s0 -s 41.1.0.0/16 -j MASQUERADE >/dev/null 2>&1
iptables -t nat -I POSTROUTING I -o enp2s0 -s 41.1.0.0/16 -j MASQUERADE >/dev/null 2>&1
