#!/bin/bash

BIN="sindicato/"
FILE="sindicato.config"
METADATA="Metadata/Metadata.AFIP"
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh serv [PUERTO_ESCUCHA]"
    echo "./config.sh mount [PUNTO_MONTAJE]"
    echo "./config.sh blk [BLOCK_SIZE] [BLOCKS]"
else
    if [ "$1" == "serv" ];then
        source pedidosya.config
        sed -i "5s/.*/MI_IP=$IP_SINDICATO/" $BIN$FILE
        sed -i "1s/.*/PUERTO_ESCUCHA=$2/" $BIN$FILE
        echo "Nuevo puerto de escucha: $2"
    elif [ "$1" == "mount" ];then
        sed -i "2s/.*/PUNTO_MONTAJE=$2/" $BIN$FILE
        echo "Nuevo punto de montaje: $2"
    elif [ "$1" == "blk" ];then
        source $BIN$FILE
        rm -rf $PUNTO_MONTAJE
        mkdir -p $PUNTO_MONTAJE/Metadata/
        touch $PUNTO_MONTAJE/$METADATA
        echo " " > $PUNTO_MONTAJE/$METADATA
        echo "Se creó por primera vez el archivo Metadata en $PUNTO_MONTAJE/$METADATA"
        sed -i "1s/.*/BLOCK_SIZE=$2\nBLOCKS=$3\nMAGIC_NUMBER=AFIP/" $PUNTO_MONTAJE/$METADATA
        echo "Nuevo tamaño de bloque: $2"
        echo "Nueva cantidad de bloques: $3"
    fi
fi
