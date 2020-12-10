#!/bin/bash

BIN="app/"
FILE="app.config"
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh com [IP_COMANDA] [PUERTO_COMANDA]"
    echo "./config.sh serv [PUERTO_ESCUCHA]"
    echo "./config.sh cpu [RETARDO_CICLO_CPU]"
    echo "./config.sh gm [GRADO_DE_MULTIPROCESAMIENTO]"
    echo "./config.sh fifo"
    echo "./config.sh hrrn [ALPHA] [ESTIMACION_INICIAL]"
    echo "./config.sh sjf [ALPHA] [ESTIMACION_INICIAL]"
    echo "./config.sh rep \"[POS_REPARTIDORES]\" [FRECUENCIA_DE_DESCANSO] [TIEMPO_DE_DESCANSO]"
else
    if [ "$1" == "com" ];then
        sed -i "1s/.*/IP_COMANDA=$2/" $BIN$FILE
        sed -i "2s/.*/PUERTO_COMANDA=$3/" $BIN$FILE
        sed "1q;d" $BIN$FILE
        sed "2q;d" $BIN$FILE
    elif [ "$1" == "serv" ];then
        sed -i "3s/.*/PUERTO_ESCUCHA=$2/" $BIN$FILE
        sed "3q;d" $BIN$FILE
    elif [ "$1" == "cpu" ];then
        sed -i "4s/.*/RETARDO_CICLO_CPU=$2/" $BIN$FILE
        sed "4q;d" $BIN$FILE
    elif [ "$1" == "gm" ];then
        sed -i "5s/.*/GRADO_DE_MULTIPROCESAMIENTO=$2/" $BIN$FILE
        sed "5q;d" $BIN$FILE
    elif [ "$1" == "fifo" ];then
        sed -i "6s/.*/ALGORITMO_DE_PLANIFICACION=FIFO/" $BIN$FILE
        sed "6q;d" $BIN$FILE
    elif [ "$1" == "hrrn" ];then
        sed -i "6s/.*/ALGORITMO_DE_PLANIFICACION=HRRN/" $BIN$FILE
        sed -i "7s/.*/ALPHA=$2/" $BIN$FILE
        sed -i "8s/.*/ESTIMACION_INICIAL=$3/" $BIN$FILE
        sed "6q;d" $BIN$FILE
        sed "7q;d" $BIN$FILE
        sed "8q;d" $BIN$FILE
    elif [ "$1" == "sjf" ];then
        sed -i "6s/.*/ALGORITMO_DE_PLANIFICACION=SJF-SD/" $BIN$FILE
        sed -i "7s/.*/ALPHA=$2/" $BIN$FILE
        sed -i "8s/.*/ESTIMACION_INICIAL=$3/" $BIN$FILE
        sed "6q;d" $BIN$FILE
        sed "7q;d" $BIN$FILE
        sed "8q;d" $BIN$FILE
    elif [ "$1" == "rep" ];then
        sed -i "9s/.*/REPARTIDORES=$2/" $BIN$FILE
        sed -i "10s/.*/FRECUENCIA_DE_DESCANSO=$3/" $BIN$FILE
        sed -i "11s/.*/TIEMPO_DE_DESCANSO=$4/" $BIN$FILE
        sed "9q;d" $BIN$FILE
        sed "10q;d" $BIN$FILE
        sed "11q;d" $BIN$FILE
    elif [ "$1" == "pl" ];then
        sed -i "13s/.*/PLATOS_DEFAULT=$2/" $BIN$FILE
        sed "13q;d" $BIN$FILE
    elif [ "$1" == "pos" ];then
        sed -i "14s/.*/POSICION_REST_DEFAULT_X=$2/" $BIN$FILE
        sed -i "15s/.*/POSICION_REST_DEFAULT_Y=$3/" $BIN$FILE
        sed "14q;d" $BIN$FILE
        sed "15q;d" $BIN$FILE
    fi
fi