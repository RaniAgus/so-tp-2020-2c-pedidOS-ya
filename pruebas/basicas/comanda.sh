source pedidosya.config

echo "---------- [   CONFIGURACIÓN DE COMANDA   ] ----------"
./comanda/config.sh serv $PUERTO_COMANDA
./comanda/config.sh mem 1024
./comanda/config.sh swp 4096
./comanda/config.sh lru
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./cliente/config.sh $IP_COMANDA $PUERTO_COMANDA