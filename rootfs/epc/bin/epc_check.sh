#!/bin/bash

sleep 10

state=0
down_cnt=0

while true
do
    num=`pgrep vepc-epcd -a | grep "\-d" -c`
    if [ $num -eq 5 ]; then
        if [ $state -eq 0 ]; then
            state=1
            down_cnt=0
        fi
    else
        if [ $state -eq 1 ]; then
            state=0
        fi
        down_cnt=$((down_cnt+1))
        if [ $down_cnt -eq 3 ]; then
            reboot
        fi
    fi
    sleep 2
done
