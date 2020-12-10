#!/bin/bash

BIN="restaurante/"
FILE="restaurante.config"
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh serv [PUERTO_ESCUCHA]"
    echo "./config.sh sin [IP_SINDICATO] [PUERTO_SINDICATO]"
    echo "./config.sh app [IP_APP] [PUERTO_APP]"
    echo "./config.sh cpu [RETARDO_CICLO_CPU]"
    echo "./config.sh fifo"
    echo "./config.sh rr [QUANTUM]"
    echo "./config.sh [NOMBRE_RESTAURANTE]"
else
    if [ "$1" == "serv" ];then
        sed -i "1s/.*/PUERTO_ESCUCHA=$2/" $BIN$FILE
        sed "1q;d" $BIN$FILE
    elif [ "$1" == "sin" ];then
        sed -i "2s/.*/IP_SINDICATO=$2/" $BIN$FILE
        sed -i "3s/.*/PUERTO_SINDICATO=$3/" $BIN$FILE
        sed "2q;d" $BIN$FILE
        sed "3q;d" $BIN$FILE
    elif [ "$1" == "app" ];then
        sed -i "4s/.*/IP_APP=$2/" $BIN$FILE
        sed -i "5s/.*/PUERTO_APP=$3/" $BIN$FILE
        sed "4q;d" $BIN$FILE
        sed "5q;d" $BIN$FILE
    elif [ "$1" == "cpu" ];then
        sed -i "6s/.*/RETARDO_CICLO_CPU=$2/" $BIN$FILE
        sed "6q;d" $BIN$FILE
    elif [ "$1" == "fifo" ];then
        sed -i "7s/.*/ALGORITMO_DE_PLANIFICACION=FIFO/" $B
        sed "7q;d" $BIN$FILE
    elif [ "$1" == "rr" ];then
        sed -i "7s/.*/ALGORITMO_DE_PLANIFICACION=RR/" $BIN$FILE
        sed -i "8s/.*/QUANTUM=$2/" $BIN$FILE
        sed "7q;d" $BIN$FILE
        sed "8q;d" $BIN$FILE
    else
        sed -i "9s/.*/NOMBRE_RESTAURANTE=$1/" $BIN$FILE
        sed -i "10s/.*/ARCHIVO_LOG=$1.log/" $BIN$FILE
        sed "9q;d" $BIN$FILE
        sed "10q;d" $BIN$FILE
    fi
fi