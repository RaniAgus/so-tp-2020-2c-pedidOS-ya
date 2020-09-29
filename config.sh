#!/bin/bash

FOLDER=""

if [ $# -lt 1 ];then
    echo "Expected:"
    echo "./config.sh <MODULE> <PARAMS>"
    echo "MODULES: cli | serv | app | com | res | sin | [SUBFOLDER]"
else
    if [ "$1" == "cli" ];then
        FOLDER=cliente

    elif [ "$1" == "serv" ];then
        FOLDER=servidor

    elif [ "$1" == "app" ];then
        FOLDER=app

    elif [ "$1" == "com" ];then
        FOLDER=comanda

    elif [ "$1" == "res" ];then
        FOLDER=restaurante

    elif [ "$1" == "sin" ];then
        FOLDER=sindicato

    else 
        FOLDER=$1
        
    fi
    cd $FOLDER
    shift;
    bash config.sh "$@"

fi