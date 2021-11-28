#!/bin/bash
iptables -t nat -F POSTROUTING
iptables -t nat -A POSTROUTING -o enp3s0 -s 10.10.0.0/16 -j MASQUERADE
iptables -F FORWARD
iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1420
