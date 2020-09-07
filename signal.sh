#!/bin/bash

if [ -n "$2" ]; then
    pid=$(pidof $2)
    if [ -n "$1 ]; then
        kill -$1 $pid
    else
        echo "falta nombre de señal"
    fi
else
    echo "falta nombre de proceso"
fi

