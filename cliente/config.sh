#!/bin/bash

BIN=""
FILE="cliente.config"
IP=""
PORT=""
CLIENT_ID=""
if [ $# -lt 2 ];then
    echo "Expected:"
    echo "./config.sh [IP] [PORT]"
    echo "./config.sh [IP] [PORT] [CLIENT_ID]"
else
    IP=$1
    PORT=$2
    CLIENT_ID=$3

    sed -i "1s/.*/IP=$IP/" $BIN$FILE
    sed -i "2s/.*/PUERTO=$PORT/" $BIN$FILE

    if [ $# -eq 3 ];then
        sed -i "6s/.*/ID_CLIENTE=Cliente$CLIENT_ID/" $BIN$FILE
    fi
fi