#!/bin/bash

time=$(date "+%Y%m%d-%H%M%S")
echo "$time $1 lost, trigger reboot" >> /epc/log/remote_check.log
echo 1 >> /epc/cfg/reboot
