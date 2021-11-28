#!/bin/bash

DST_URL=https://oapi.dingtalk.com/robot/send?access_token=42d7a9eb4ddc4439c8d34403ab85f7b5529eac5838f3aff210854ec6e140057f

fail_cnt=0
while true
do
    ping -c 2 -W 1 $1 > /dev/null
    if [ $? != 0 ];then
        fail_cnt=$((fail_cnt + 1))
        time_now=$(date "+%Y%m%d_%H:%M:%S")
        curl $DST_URL -H 'Content-type:application/json' -d '{"msgtype":"text","text":{"content":"'EPC:\ $time_now\ 江南电缆厂探测UE下线\($fail_cnt\)!!!!!!'"}}'
    else
        if [ $fail_cnt -ne 0 ];then
            time_now=$(date "+%Y%m%d_%H:%M:%S")
            curl $DST_URL -H 'Content-type:application/json' -d '{"msgtype":"text","text":{"content":"'EPC:\ $time_now\ 江南电缆厂探测UE上线!'"}}'
        fi
        fail_cnt=0
    fi
    sleep 6s
done
