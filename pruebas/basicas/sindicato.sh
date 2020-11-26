source pedidosya.config

echo "---------- [  CONFIGURACIÓN DE SINDICATO  ] ----------"
./sindicato/config.sh serv $PUERTO_SINDICATO
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./cliente/config.sh $IP_SINDICATO $PUERTO_SINDICATO