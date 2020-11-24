source pedidosya.config

echo "---------- [   CONFIGURACIÓN DE COMANDA   ] ----------"
./config.sh com serv $PUERTO_COMANDA
./config.sh com mem 1024
./config.sh com swp 4096
./config.sh com lru
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./config.sh cli $IP_COMANDA $PUERTO_COMANDA