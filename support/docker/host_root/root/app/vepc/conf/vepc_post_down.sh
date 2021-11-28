#!/bin/bash

#ifconfig enp2s0 down
#ifconfig enp3s0 down
#ifconfig enp6s0f0 down

#iptables -t nat -F POSTROUTING
#iptables -t nat -A POSTROUTING -o enp6s0f0 -s 10.10.0.0/16 -j MASQUERADE
#iptables -F FORWARD
#iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1420
