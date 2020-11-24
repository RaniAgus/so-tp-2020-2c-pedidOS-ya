#!/bin/bash

BIN=""
FILE="cliente.config"
IP=""
PORT=""
CLIENT_ID=""
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh [IP] [PUERTO]"
    echo "./config.sh pos [POSICION_X] [POSICION_Y]"
    echo "./config.sh [ID_CLIENTE]"
else
    if [ $# -eq 1 ];then
        sed -i "3s/.*/ARCHIVO_LOG=cliente$1.log/"
        sed -i "6s/.*/ID_CLIENTE=Cliente$1/" $BIN$FILE
    elif [ $# -eq 2 ];then
        sed -i "1s/.*/IP=$1/" $BIN$FILE
        sed -i "2s/.*/PUERTO=$2/" $BIN$FILE
    elif [ $# -eq 3 ];then
        sed -i "4s/.*/POSICION_X=$2/" $BIN$FILE
        sed -i "5s/.*/POSICION_Y=$3/" $BIN$FILE
        echo "Nueva posicion: [$2,$3]"
    fi
fi