#! /bin/bash

DATE=`date -u`
ssh root@192.168.7.2 "date -s \"$DATE\""
