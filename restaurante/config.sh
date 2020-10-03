#!/bin/bash

BIN=""
FILE="restaurante.config"
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh serv [PUERTO_ESCUCHA]"
    echo "./config.sh sin [IP_SINDICATO] [PUERTO_SINDICATO]"
    echo "./config.sh app [IP_APP] [PUERTO_APP]"
    echo "./config.sh fifo"
    echo "./config.sh rr [QUANTUM]"
    echo "./config.sh [NOMBRE_RESTAURANTE]"
else
    if [ "$1" == "serv" ];then
        sed -i "1s/.*/PUERTO_ESCUCHA=$2/" $BIN$FILE
        echo "Nuevo puerto de escucha: $2"
    elif [ "$1" == "sin" ];then
        sed -i "2s/.*/IP_SINDICATO=$2/" $BIN$FILE
        sed -i "3s/.*/PUERTO_SINDICATO=$3/" $BIN$FILE
        echo "Nueva ip de sindicato: $2"
        echo "Nuevo puerto de sindicato: $3"
    elif [ "$1" == "app" ];then
        sed -i "4s/.*/IP_APP=$2/" $BIN$FILE
        sed -i "5s/.*/PUERTO_APP=$3/" $BIN$FILE
        echo "Nueva ip de app: $2"
        echo "Nuevo puerto de app: $3"
    elif [ "$1" == "fifo" ];then
        sed -i "6s/.*/ALGORITMO_DE_PLANIFICACION=FIFO/" $BIN$FILE
        echo "Nuevo algoritmo de planificacion: FIFO"
    elif [ "$1" == "rr" ];then
        sed -i "6s/.*/ALGORITMO_DE_PLANIFICACION=RR/" $BIN$FILE
        sed -i "7s/.*/QUANTUM=$2/" $BIN$FILE
        echo "Nuevo algoritmo de planificacion: RR"
        echo "Nuevo quantum: $2"
    else
        sed -i "8s/.*/NOMBRE_RESTAURANTE=$1/" $BIN$FILE
        sed -i "9s/.*/ARCHIVO_LOG=$1.log/" $BIN$FILE
        echo "Nuevo nombre de restaurante: $1"
    fi
fi