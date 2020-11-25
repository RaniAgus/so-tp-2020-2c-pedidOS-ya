source pedidosya.config

echo "---------- [  CONFIGURACIÓN DE SINDICATO  ] ----------"
./sindicato/config.sh serv $PUERTO_SINDICATO

echo "---------- [ CONFIGURACIÓN DE RESTAURANTE ] ----------"
./restaurante/config.sh sin $IP_SINDICATO $PUERTO_SINDICATO
./restaurante/config.sh serv $PUERTO_RESTAURANTE
./restaurante/config.sh ElParrillon
./restaurante/config.sh fifo
./restaurante/config.sh cpu 2 # Valor original: 5
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./cliente/config.sh $IP_RESTAURANTE $PUERTO_RESTAURANTE

read -p ">> Presiona enter para configurar la siguiente prueba (rr)"
echo "---------- [ CONFIGURACIÓN DE RESTAURANTE ] ----------"
./restaurante/ res rr 2