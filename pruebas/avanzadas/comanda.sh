source pedidosya.config

echo "---------- [   CONFIGURACIÓN DE COMANDA   ] ----------"
./config.sh com serv $PUERTO_COMANDA
./config.sh com mem 128
./config.sh com swp 4096
./config.sh com lru
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./config.sh cli $IP_COMANDA $PUERTO_COMANDA
echo "------------------------------------------------------"
read -p ">> Presiona enter para configurar la siguiente prueba"
echo "---------- [   CONFIGURACIÓN DE COMANDA   ] ----------"
./config.sh com serv $PUERTO_COMANDA
./config.sh com clk