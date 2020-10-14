#!/bin/bash

BIN=""
FILE="sindicato.config"
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh serv [PUERTO_ESCUCHA]"
    echo "./config.sh mount [PUNTO_MONTAJE]"
else
    if [ "$1" == "serv" ];then
        sed -i "1s/.*/PUERTO_ESCUCHA=$2/" $BIN$FILE
        echo "Nuevo puerto de escucha: $2"
    elif [ "$1" == "mount" ];then
        sed -i "2s/.*/PUNTO_MONTAJE=$2/" $BIN$FILE
        echo "Nuevo punto de montaje: $2"
    fi
fi