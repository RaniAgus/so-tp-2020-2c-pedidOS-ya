#!/bin/bash
source pedidosya.config

echo "---------- [   CONFIGURACIÓN DE COMANDA   ] ----------"
./comanda/config.sh serv $PUERTO_COMANDA
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./app/config.sh com $IP_COMANDA $PUERTO_COMANDA
./app/config.sh serv $PUERTO_APP
./app/config.sh cpu 4
./app/config.sh gm 1
./app/config.sh fifo
./app/config.sh pl [Milanesa]
./app/config.sh pos 5 5
./app/config.sh rep "[6|4,5|7,1|1]" [15,15,15] [1,1,1]
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./cliente/config.sh $IP_APP $PUERTO_APP
./cliente/config.sh id 1
./cliente/config.sh pos 4 4
echo "------------------------------------------------------"

read -p ">> Presiona enter para configurar la siguiente prueba (sjf)"
./app/config.sh sjf 0.5 2

read -p ">> Presiona enter para configurar la siguiente prueba (hrrn)"
./app/config.sh hrrn 0.5 2
