source pedidosya.config

function configurarClientes {
    read -p ">> Presiona enter para configurar al Cliente1"
    ./config.sh cli 1
    ./config.sh cli pos 1 9
    read -p ">> Presiona enter para configurar al Cliente2"
    ./config.sh cli 2
    ./config.sh cli pos 2 4
    read -p ">> Presiona enter para configurar al Cliente3"
    ./config.sh cli 3
    ./config.sh cli pos 2 1
}
echo "---------- [   CONFIGURACIÓN DE COMANDA   ] ----------"
./config.sh com serv $PUERTO_COMANDA
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./config.sh app com $IP_COMANDA $PUERTO_COMANDA
./config.sh app serv $PUERTO_APP
./config.sh app cpu 4
./config.sh app gm 1
./config.sh app fifo
./config.sh app rep "[1|1,7|8,1|4]" [50,50,90] [2,2,1]
./config.sh app pl [Milanesa]
./config.sh app pos 5 5
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./config.sh cli $IP_APP $PUERTO_APP
configurarClientes
echo "------------------------------------------------------"
read -p ">> Presiona enter para configurar la siguiente prueba"
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./config.sh app rep "[1|1,7|8,1|4]" [5,5,9] [2,2,1]
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
configurarClientes