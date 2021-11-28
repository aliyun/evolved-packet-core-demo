#!/bin/bash

temp=`vmstat 1 2 |tail -1`
cpu_idle=`echo $temp |awk '{printf("%s\n",$15)}'`


disk_ava=$(df / | awk '/\//{print $4}')
free_mem=$(free | awk '/Mem/{print $4}')

if [ $cpu_idle -le 5 -o $disk_ava -le 2048000 -o $free_mem -le 204800 ]; then
    time=$(date "+%Y-%m-%d %H:%M:%S")
    echo "$time idle=$cpu_idle, disk_ava_size=$disk_ava, free_mem_size=$free_mem" >> /epc/log/vepc.log
    sleep 1s
    reboot
fi
