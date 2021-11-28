#!/bin/bash

sleep 10

state=0
down_cnt=0
up_cnt=0

while true
do
    num=`pgrep vepc- -c`
    if [ $num -eq 4 ]; then
        if [ $state -eq 0 ]; then
            state=1
            down_cnt=0
        fi
        up_cnt=$((up_cnt+1))
    else
        if [ $state -eq 1 ]; then
            state=0
        fi
        down_cnt=$((down_cnt+1))
        if [ $down_cnt -gt 2 ]; then
            /epc/bin/vepc_down.sh
            killall -9 keepalived
            if [ $up_cnt -gt 30 ]; then
                echo 1 > /epc/cfg/reboot
            fi
        fi
    fi
    sleep 2
done
