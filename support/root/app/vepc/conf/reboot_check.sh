#!/bin/bash
sleep 1m

while true
do
    if [ -f "/root/app/vepc/conf/reboot" ];then
        rm -f /root/app/vepc/conf/reboot
        reboot
    fi
    sleep 5s
done
