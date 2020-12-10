source pedidosya.config

echo "---------- [  CONFIGURACIÓN DE SINDICATO  ] ----------"
./sindicato/config.sh serv $PUERTO_SINDICATO
./sindicato/config.sh blk 64 2048
echo "---------- [   CONFIGURACIÓN DE COMANDA   ] ----------"
./comanda/config.sh serv $PUERTO_COMANDA
./comanda/config.sh mem 4096
./comanda/config.sh swp 4096
./comanda/config.sh lru
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./app/config.sh com $IP_COMANDA $PUERTO_COMANDA
./app/config.sh serv $PUERTO_APP
./app/config.sh cpu 4
./app/config.sh gm 2
./app/config.sh fifo
./app/config.sh rep "[7|3,3|5]" [20,8] [1,2]
./app/config.sh pl [Milanesa]
./app/config.sh pos 5 5
echo "---------- [ CONFIGURACIÓN DE RESTAURANTE ] ----------"
./restaurante/config.sh sin $IP_SINDICATO $PUERTO_SINDICATO
./restaurante/config.sh app $IP_APP $PUERTO_APP
./restaurante/config.sh cpu 5

read -p ">> Presiona enter para configurar el siguiente restaurante (la parri)"
echo "---------- [ CONFIGURACIÓN DE RESTAURANTE ] ----------"
./restaurante/config.sh serv $PUERTO_RESTAURANTE
./restaurante/config.sh LaParri
./restaurante/config.sh rr 2

read -p ">> Presiona enter para configurar el siguiente restaurante (milanga palace)"
echo "---------- [ CONFIGURACIÓN DE RESTAURANTE ] ----------"
./restaurante/config.sh serv $PUERTO_RESTAURANTE2
./restaurante/config.sh MilangaPalace
./restaurante/config.sh fifo

read -p ">> Presiona enter para configurar el siguiente restaurante (green life)"
echo "---------- [ CONFIGURACIÓN DE RESTAURANTE ] ----------"
./restaurante/config.sh serv $PUERTO_RESTAURANTE3
./restaurante/config.sh GreenLife
./restaurante/config.sh fifo

echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./cliente/config.sh $IP_APP $PUERTO_APP
read -p ">> Presiona enter para configurar al Cliente1"
./cliente/config.sh id 1
./cliente/config.sh pos 2 2
read -p ">> Presiona enter para configurar al Cliente2"
./cliente/config.sh id 2
./cliente/config.sh pos 6 6
read -p ">> Presiona enter para configurar al Cliente3"
./cliente/config.sh id 3
./cliente/config.sh pos 8 9