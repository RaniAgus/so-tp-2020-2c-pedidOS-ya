#!/bin/bash

BIN=""
FILE="server.config"
MODULE=""
if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh [MODULE]"
else
    MODULE=$1

    sed -i "1s/.*/MODULO=$MODULE/" $BIN$FILE
fi