source pedidosya.config

echo "---------- [  CONFIGURACIÓN DE SINDICATO  ] ----------"
./sindicato/config.sh serv $PUERTO_SINDICATO
./sindicato/config.sh blk 64 1024

echo "---------- [ CONFIGURACIÓN DE RESTAURANTE ] ----------"
./restaurante/config.sh sin $IP_SINDICATO $PUERTO_SINDICATO
./restaurante/config.sh serv $PUERTO_RESTAURANTE
./restaurante/config.sh ElParrillon
./restaurante/config.sh fifo
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./cliente/config.sh $IP_RESTAURANTE $PUERTO_RESTAURANTE