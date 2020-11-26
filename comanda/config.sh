#!/bin/bash

BIN=""
FILE="comanda/comanda.config"
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh serv [PUERTO_ESCUCHA]"
    echo "./config.sh mem [TAMANIO_MEMORIA]"
    echo "./config.sh swp [TAMANIO_SWAP]"
    echo "./config.sh clk"
    echo "./config.sh lru"
else
    if [ "$1" == "serv" ];then
        sed -i "1s/.*/PUERTO_ESCUCHA=$2/" $BIN$FILE
        echo "Nuevo puerto de escucha: $2"
    elif [ "$1" == "mem" ];then
        sed -i "2s/.*/TAMANIO_MEMORIA=$2/" $BIN$FILE
        echo "Nuevo tamaño de memoria: $2"
    elif [ "$1" == "swp" ];then
        sed -i "3s/.*/TAMANIO_SWAP=$2/" $BIN$FILE
        echo "Nuevo tamaño de swap: $2"
    elif [ "$1" == "clk" ];then
        sed -i "4s/.*/ALGORITMO_REEMPLAZO=CLOCK/" $BIN$FILE
        echo "Nuevo algoritmo de reemplazo: CLOCK"
    elif [ "$1" == "lru" ];then
        sed -i "4s/.*/ALGORITMO_REEMPLAZO=LRU/" $BIN$FILE
        echo "Nuevo algoritmo de reemplazo: LRU"
    fi
fi