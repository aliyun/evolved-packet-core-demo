#!/bin/bash
DATE_STR=$(date '+%Y%m%d')
LOG_DIR=/epc/log

LOG_FILE_DIR=/epc/log
LOG_FILE=/epc/log/vepc.log
LOG_GZ_FILE=$LOG_DIR/$DATE_STR.tar.gz
tar -czf $LOG_GZ_FILE -C $LOG_FILE_DIR vepc.log
echo $DATE_STR > $LOG_FILE_DIR/vepc.log

DATE_STR_2=$(date -d '30 days ago' '+%Y%m%d')
FILE=$LOG_DIR/$DATE_STR_2.tar.gz
if [ -f $FILE ];then
    rm $FILE
fi
