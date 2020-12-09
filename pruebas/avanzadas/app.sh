source pedidosya.config

function configurarClientes {
    read -p ">> Presiona enter para configurar al Cliente1"
    ./cliente/config.sh 1
    ./cliente/config.sh pos 1 9
    read -p ">> Presiona enter para configurar al Cliente2"
    ./cliente/config.sh 2
    ./cliente/config.sh pos 4 4
    read -p ">> Presiona enter para configurar al Cliente3"
    ./cliente/config.sh 3
    ./cliente/config.sh pos 2 1
}

function probarSinDescanso {
    ./app/config.sh rep "[1|1,7|8,1|4]" [50,50,90] [1,1,1]
}

function probarConDescanso {
    echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
    ./app/config.sh rep "[1|1,7|8,1|4]" [4,6,10] [1,2,3]
    echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
    configurarClientes
}

# Primera prueba: FIFO - Repartidores sin descanso

echo "---------- [   CONFIGURACIÓN DE COMANDA   ] ----------"
./comanda/config.sh serv $PUERTO_COMANDA
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./app/config.sh com $IP_COMANDA $PUERTO_COMANDA
./app/config.sh serv $PUERTO_APP
./app/config.sh cpu 4
./app/config.sh gm 2
./app/config.sh fifo
./app/config.sh pl [Milanesa]
./app/config.sh pos 5 5
probarSinDescanso
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
./cliente/config.sh $IP_APP $PUERTO_APP
configurarClientes
echo "------------------------------------------------------"

# Segunda prueba: FIFO - Repartidores con descanso

read -p ">> Presiona enter para configurar la siguiente prueba (fifo - descanso)"
probarConDescanso

# Tercera prueba: SJF - Repartidores sin descanso

read -p ">> Presiona enter para configurar la siguiente prueba (sjf)"
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./app/config.sh sjf 0.5 5
probarSinDescanso
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
configurarClientes

# Cuarta prueba: SJF - Repartidores con descanso

read -p ">> Presiona enter para configurar la siguiente prueba (sjf - descanso)"
probarConDescanso

# Quinta prueba: HRRN - Repartidores sin descanso

read -p ">> Presiona enter para configurar la siguiente prueba (hrrn)"
echo "---------- [     CONFIGURACIÓN DE APP     ] ----------"
./app/config.sh hrrn 0.5 5
probarSinDescanso
echo "---------- [   CONFIGURACIÓN DE CLIENTE   ] ----------"
configurarClientes

# Sexta prueba: HRRN - Repartidores con descanso

read -p ">> Presiona enter para configurar la siguiente prueba (hrrn - descanso)"
probarConDescanso