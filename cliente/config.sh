#!/bin/bash

BIN=""
FILE="cliente.config"
IP=""
PORT=""
CLIENT_ID=""
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh [IP] [PUERTO]"
    echo "./config.sh [ID_CLIENTE]"
else
    if [ $# -eq 1 ];then
        sed -i "6s/.*/ID_CLIENTE=Cliente$1/" $BIN$FILE
    elif [ $# -eq 2 ];then
        sed -i "1s/.*/IP=$1/" $BIN$FILE
        sed -i "2s/.*/PUERTO=$2/" $BIN$FILE
    fi
fi