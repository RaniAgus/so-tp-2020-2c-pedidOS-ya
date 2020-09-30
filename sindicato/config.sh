#!/bin/bash

BIN=""
FILE="sindicato.config"
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh serv [PUERTO_ESCUCHA]"
    echo "./config.sh mount [PUNTO_MONTAJE_TALLGRASS]"
    echo "./config.sh blocks [BLOCKS]"
    echo "./config.sh size [BLOCKS_SIZE]"
else
    if [ "$1" == "serv" ];then
        sed -i "6s/.*/PUERTO_ESCUCHA=$2/" $BIN$FILE
        echo "Nuevo puerto de escucha: $2"
    elif [ "$1" == "mount" ];then
        sed -i "3s/.*/PUNTO_MONTAJE_TALLGRASS=$2/" $BIN$FILE
        echo "Nuevo punto de montaje: $2"
    elif [ "$1" == "blocks" ];then
        sed -i "4s/.*/BLOCKS=$2/" $BIN$FILE
        echo "Nueva cantidad de bloques: $2"
    elif [ "$1" == "size" ];then
        sed -i "5s/.*/BLOCKS_SIZE=$2/" $BIN$FILE
        echo "Nuevo tamaño de bloque: $2"
    fi
fi