#!/bin/bash

BIN=""
FILE="app.config"
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh com [IP_COMANDA] [PUERTO_COMANDA]"
    echo "./config.sh serv [PUERTO_ESCUCHA]"
    echo "./config.sh gm [GRADO_DE_MULTIPROCESAMIENTO]"
    echo "./config.sh fifo"
    echo "./config.sh hrrn [ALPHA] [ESTIMACION_INICIAL]"
    echo "./config.sh sjf [ALPHA] [ESTIMACION_INICIAL]"
else
    if [ "$1" == "com" ];then
        sed -i "1s/.*/IP_COMANDA=$2/" $BIN$FILE
        sed -i "2s/.*/PUERTO_COMANDA=$3/" $BIN$FILE
        echo "Nueva ip de comanda: $2"
        echo "Nuevo puerto de comanda: $3"
    elif [ "$1" == "serv" ];then
        sed -i "3s/.*/PUERTO_ESCUCHA=$2/" $BIN$FILE
        echo "Nuevo puerto de escucha: $2"
    elif [ "$1" == "gm" ];then
        sed -i "4s/.*/GRADO_DE_MULTIPROCESAMIENTO=$2/" $BIN$FILE
        echo "Nuevo grado de multiprocesamiento: $2"
    elif [ "$1" == "fifo" ];then
        sed -i "5s/.*/ALGORITMO_DE_PLANIFICACION=FIFO/" $BIN$FILE
        echo "Nuevo algoritmo de planificacion: FIFO"
    elif [ "$1" == "hrrn" ];then
        sed -i "5s/.*/ALGORITMO_DE_PLANIFICACION=HRRN/" $BIN$FILE
        sed -i "6s/.*/ALPHA=$2/" $BIN$FILE
        sed -i "7s/.*/ESTIMACION_INICIAL=$3/" $BIN$FILE
        echo "Nuevo algoritmo de planificacion: HRRN"
        echo "Nuevo alpha: $2"
        echo "Nueva estimacion inicial: $3"
    elif [ "$1" == "sjf" ];then
        sed -i "5s/.*/ALGORITMO_DE_PLANIFICACION=SJF/" $BIN$FILE
        sed -i "6s/.*/ALPHA=$2/" $BIN$FILE
        sed -i "7s/.*/ESTIMACION_INICIAL=$3/" $BIN$FILE
        echo "Nuevo algoritmo de planificacion: SJF"
        echo "Nuevo alpha: $2"
        echo "Nueva estimacion inicial: $3"
    fi
fi