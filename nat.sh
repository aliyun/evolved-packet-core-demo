#!/bin/bash
iptables -t nat -F POSTROUTING
iptables -t nat -A POSTROUTING -o enp1s0 -s 45.45.0.0/16 -j MASQUERADE
iptables -t nat -A POSTROUTING -o enp0s3 -s 42.1.0.0/16 -j MASQUERADE
iptables -F FORWARD
iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --set-mss 1420
