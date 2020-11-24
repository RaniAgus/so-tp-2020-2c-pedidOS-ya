source pedidosya.config

function configurarClientes {
    read -p ">> Presiona enter para configurar al Cliente1"
    ./config.sh cli 1
    ./config.sh cli pos 1 9
    read -p ">> Presiona enter para configurar al Cliente2"
    ./config.sh cli 2
    ./config.sh cli pos 4 4
    read -p ">> Presiona enter para configurar al Cliente3"
    ./config.sh cli 3
    ./config.sh cli pos 2 1
}

function probarSinDescanso {
    ./config.sh app rep "[1|1,7|8,1|4]" [50,50,90] [1,1,1]
}

function probarConDescanso {
    echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
    ./config.sh app rep "[1|1,7|8,1|4]" [5,8,13] [1,2,3]
    echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
    configurarClientes
}

# Primera prueba: FIFO - Repartidores sin descanso

echo "---------- [   CONFIGURACIÓN DE COMANDA   ] ----------"
./config.sh com serv $PUERTO_COMANDA
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./config.sh app com $IP_COMANDA $PUERTO_COMANDA
./config.sh app serv $PUERTO_APP
./config.sh app cpu 4
./config.sh app gm 2
./config.sh app fifo
./config.sh app pl [Milanesa]
./config.sh app pos 5 5
probarSinDescanso
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./config.sh cli $IP_APP $PUERTO_APP
configurarClientes
echo "------------------------------------------------------"

# Segunda prueba: FIFO - Repartidores con descanso

read -p ">> Presiona enter para configurar la siguiente prueba (fifo - descanso)"
probarConDescanso

# Tercera prueba: SJF - Repartidores sin descanso

read -p ">> Presiona enter para configurar la siguiente prueba (sjf)"
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./config.sh app sjf 0.5 5
probarSinDescanso
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
configurarClientes

# Cuarta prueba: SJF - Repartidores con descanso

read -p ">> Presiona enter para configurar la siguiente prueba (sjf - descanso)"
probarConDescanso

# Quinta prueba: HRRN - Repartidores sin descanso

read -p ">> Presiona enter para configurar la siguiente prueba (hrrn)"
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./config.sh app hrrn 0.5 5
probarSinDescanso
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
configurarClientes

# Sexta prueba: HRRN - Repartidores con descanso

read -p ">> Presiona enter para configurar la siguiente prueba (hrrn - descanso)"
probarConDescanso