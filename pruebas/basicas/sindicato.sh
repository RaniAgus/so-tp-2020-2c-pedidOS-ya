source pedidosya.config

echo "---------- [  CONFIGURACIÓN DE SINDICATO  ] ----------"
./config.sh sin serv $PUERTO_SINDICATO
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./config.sh cli $IP_SINDICATO $PUERTO_SINDICATO