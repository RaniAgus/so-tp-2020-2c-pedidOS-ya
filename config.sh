#!/bin/bash

BIN="cliente/"
FILE="cliente.config"
IP=""
PORT=""
if [ $# -eq 0 ];then
    echo "Expected:"
    echo "./config.sh [IP] [PORT]"
else
    IP=$1
    PORT=$2

    sed -i "1s/.*/IP=$IP/" $BIN$FILE
    sed -i "2s/.*/PUERTO=$PORT/" $BIN$FILE
fi