source pedidosya.config

echo "---------- [  CONFIGURACIÓN DE SINDICATO  ] ----------"
./config.sh sin serv $PUERTO_SINDICATO

echo "---------- [ CONFIGURACIÓN DE RESTAURANTE ] ----------"
./config.sh res sin $IP_SINDICATO $PUERTO_SINDICATO
./config.sh res serv $PUERTO_RESTAURANTE
./config.sh res ElParrillon
./config.sh res fifo
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./config.sh cli $IP_RESTAURANTE $PUERTO_RESTAURANTE