source pedidosya.config

function configurarClientes {
    read -p ">> Presiona enter para configurar al Cliente1"
    ./cliente/config.sh 1
    ./cliente/config.sh pos 1 9
    read -p ">> Presiona enter para configurar al Cliente2"
    ./cliente/config.sh 2
    ./cliente/config.sh pos 2 4
    read -p ">> Presiona enter para configurar al Cliente3"
    ./cliente/config.sh 3
    ./cliente/config.sh pos 2 1
}
echo "---------- [   CONFIGURACIÓN DE COMANDA   ] ----------"
./comanda/config.sh serv $PUERTO_COMANDA
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./app/config.sh com $IP_COMANDA $PUERTO_COMANDA
./app/config.sh serv $PUERTO_APP
./app/config.sh cpu 4
./app/config.sh gm 1
./app/config.sh fifo
./app/config.sh rep "[1|1,7|8,1|4]" [50,50,90] [2,2,1]
./app/config.sh pl [Milanesa]
./app/config.sh pos 5 5
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./cliente/config.sh $IP_APP $PUERTO_APP
configurarClientes
echo "------------------------------------------------------"
read -p ">> Presiona enter para configurar la siguiente prueba"
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./app/config.sh rep "[1|1,7|8,1|4]" [5,5,9] [2,2,1]
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
configurarClientes