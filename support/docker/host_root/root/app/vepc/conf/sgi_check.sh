#!/bin/bash

gw_num=1
gw1_state=1
gw2_state=1
gw1=10.66.251.62
gw2=10.66.251.58
while true
do
    ping -c 2 $gw1 > /dev/null
    if [ $? != 0 ];then
        echo "$ip is unavialable"
        gw1_state=0
	if [ $gw2_state -eq 1 ];then
	route add default gw $gw2 dev enp2s0 >/dev/null 2>&1
	route del default gw $gw1 dev enp2s0 >/dev/null 2>&1
	gw_num=2
	time=$(date "+%Y%m%d-%H%M%S")
	echo "$time switch gw $gw2" >> /epc/log/sgi_check.log
	else
	gw_num=0
	time=$(date "+%Y%m%d-%H%M%S")
	echo "$time lost gw $gw1" >> /epc/log/sgi_check.log
	fi
    else
        gw1_state=1
	if [ $gw_num -eq 0 ];then
	route del default gw $gw1 dev enp2s0 >/dev/null 2>&1
	route add default gw $gw1 dev enp2s0 >/dev/null 2>&1
	gw_num=1
	time=$(date "+%Y%m%d-%H%M%S")
	echo "$time switch gw $gw1" >> /epc/log/sgi_check.log
	fi
    fi
    
    sleep 3s

    ping -c 2 $gw2 > /dev/null
    if [ $? != 0 ];then
        gw2_state=0
	if [ $gw1_state -eq 1 ];then
	route add default gw $gw1 dev enp2s0 >/dev/null 2>&1
	route del default gw $gw2 dev enp2s0 >/dev/null 2>&1
	gw_num=1
	time=$(date "+%Y%m%d-%H%M%S")
	echo "$time switch gw $gw1" >> /epc/log/sgi_check.log
	else
	gw_num=0
	time=$(date "+%Y%m%d-%H%M%S")
	echo "$time lost gw $gw2" >> /epc/log/sgi_check.log
	fi
    else
        gw2_state=1
	if [ $gw_num -eq 0 ];then
	route del default gw $gw2 dev enp2s0 >/dev/null 2>&1
	route add default gw $gw2 dev enp2s0 >/dev/null 2>&1
	gw_num=2
	time=$(date "+%Y%m%d-%H%M%S")
	echo "$time switch gw $gw2" >> /epc/log/sgi_check.log
	fi
    fi

done
