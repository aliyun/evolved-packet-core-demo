#!/bin/bash
sleep 1m
ipsec restart > /dev/null

ip=172.16.210.206
while true
do
    ping -c 2 -W 3 $ip > /dev/null
    if [ $? != 0 ];then
        #echo "$ip is unavialable"
        ipsec up vpn_cli > /dev/null
    fi
    sleep 3m
done
