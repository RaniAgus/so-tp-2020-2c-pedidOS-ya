#!/bin/bash

BIN="cliente/"
FILE="cliente.config"

if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh [IP] [PUERTO]"
    echo "./config.sh pos [POSICION_X] [POSICION_Y]"
    echo "./config.sh id [ID_CLIENTE]"
else
    if [ "$1" == "id" ];then
        sed -i "3s/.*/ARCHIVO_LOG=cliente$2.log/" $BIN$FILE
        sed -i "6s/.*/ID_CLIENTE=Cliente$2/" $BIN$FILE
        sed "3q;d" $BIN$FILE
        sed "6q;d" $BIN$FILE
    elif [ "$1" == "pos" ];then
        sed -i "4s/.*/POSICION_X=$2/" $BIN$FILE
        sed -i "5s/.*/POSICION_Y=$3/" $BIN$FILE
        sed "4q;d" $BIN$FILE
        sed "5q;d" $BIN$FILE
    else
        sed -i "1s/.*/IP=$1/" $BIN$FILE
        sed -i "2s/.*/PUERTO=$2/" $BIN$FILE
        sed "1q;d" $BIN$FILE
        sed "2q;d" $BIN$FILE
    fi
fi