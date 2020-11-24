source pedidosya.config

echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./config.sh app com $IP_COMANDA $PUERTO_COMANDA
./config.sh app serv $PUERTO_APP
./config.sh app cpu 4
./config.sh app gm 2
./config.sh app fifo
./config.sh app rep "[7|3,3|5]" [20,8] [1,2]
./config.sh app pl [Milanesa]
./config.sh app pos 5 5
echo "---------- [ CONFIGURACIÓN DE RESTAURANTE ] ----------"
./config.sh res sin $IP_SINDICATO $PUERTO_SINDICATO
./config.sh res serv $PUERTO_RESTAURANTE
./config.sh res LaParri
./config.sh res rr 2
./config.sh res cpu 2 # Valor original: ??

read -p ">> Presiona enter para configurar el siguiente restaurante (green life)"
echo "---------- [ CONFIGURACIÓN DE RESTAURANTE ] ----------"
./config.sh res GreenLife
./config.sh res fifo

echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./config.sh cli $IP_APP $PUERTO_APP
read -p ">> Presiona enter para configurar al Cliente1"
./config.sh cli 1
./config.sh cli pos 2 2
read -p ">> Presiona enter para configurar al Cliente2"
./config.sh cli 2
./config.sh cli pos 6 6
read -p ">> Presiona enter para configurar al Cliente3"
./config.sh cli 3
./config.sh cli pos 8 9