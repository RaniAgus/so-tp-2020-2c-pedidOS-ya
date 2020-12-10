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
        sed -i "1s/.*/PUERTO_ESCUCHA=$2/" $BIN$FILE
        sed "1q;d" $BIN$FILE
    elif [ "$1" == "mount" ];then
        sed -i "2s/.*/PUNTO_MONTAJE=$2/" $BIN$FILE
        sed "2q;d" $BIN$FILE
    elif [ "$1" == "blk" ];then
        source $BIN$FILE
        rm -rf $PUNTO_MONTAJE
        mkdir -pv $PUNTO_MONTAJE/Metadata/
        touch $PUNTO_MONTAJE/$METADATA
        echo " " > $PUNTO_MONTAJE/$METADATA
        echo "$PUNTO_MONTAJE/$METADATA:"
        sed -i "1s/.*/BLOCK_SIZE=$2\nBLOCKS=$3\nMAGIC_NUMBER=AFIP/" $PUNTO_MONTAJE/$METADATA
        sed "1q;d" $PUNTO_MONTAJE/$METADATA
        sed "2q;d" $PUNTO_MONTAJE/$METADATA
        sed "3q;d" $PUNTO_MONTAJE/$METADATA
    fi
fi
