#include "applanificador.h"

static pthread_t thread_planificador;

static t_queue*			pcbs_nuevos;
static pthread_mutex_t 	pcbs_nuevos_mutex;
static sem_t 			pcbs_nuevos_sem;

static t_list* 			repartidores_libres;
static pthread_mutex_t 	repartidores_libres_mutex;
static sem_t			repartidores_libres_sem;

static void app_asignar_repartidor(t_pcb* pcb);

void app_iniciar_planificador(void)
{
	pcbs_nuevos = queue_create();
	pthread_mutex_init(&pcbs_nuevos_mutex, NULL);
	sem_init(&pcbs_nuevos_sem, 0, 0);

	repartidores_libres = list_create();
	pthread_mutex_init(&repartidores_libres_mutex, NULL);
	sem_init(&repartidores_libres_sem, 0, 0);

	//Creo el hilo que se encarga de asignar los repartidores a cada pcb nuevo
	pthread_create(&thread_planificador, NULL, (void*) app_asignar_repartidor, NULL);
	pthread_detach(thread_planificador);

	//TODO: [APP] Crear e iniciar las listas de ready, bloqueado por descanso y bloqueado por espera
}

//Al inicio se agregan todos los repartidores creados a la lista de disponibles
void app_agregar_repartidor_disponible(t_repartidor* repartidor)
{
	pthread_mutex_lock(&repartidores_libres_mutex);
	list_add(repartidores_libres, repartidor);
	pthread_mutex_unlock(&repartidores_libres_mutex);

	sem_post(&repartidores_libres_sem);
}

//Al confirmar pedido, se crea el PCB correspondiente y se agrega a la lista de pcbs nuevos
void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id)
{
    CS_LOG_TRACE(
        "Se va a crear el PCB: {CLIENTE: %s} {RESTAURANTE: %s} {ID_PEDIDO: %d}", 
        cliente, restaurante, pedido_id
    );

    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->id_pedido = pedido_id;
    pcb->cliente = cliente;
    pcb->posicionCliente = app_posicion_cliente(cliente);
    pcb->restaurante = restaurante;
    pcb->posicionRestaurante = app_posicion_restaurante(restaurante);
    pcb->destino = DESTINO_RESTAURANTE;

    //Arranca sin repartidor asignado, otro hilo lo extrae de la cola y le asigna uno
    pcb->repartidor = NULL;
    queue_sync_push(pcbs_nuevos, &pcbs_nuevos_mutex, &pcbs_nuevos_sem, pcb);
}

//Luego de recibir Plato Listo, si el pedido está terminado se avisa al repartidor
void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id)
{
    CS_LOG_TRACE(
        "Se va a avisar al repartidor correspondiente que el pedido está terminado: {RESTAURANTE: %s} {ID_PEDIDO: %d}",
		restaurante, pedido_id
    );

    //TODO: [APP] Buscar el PCB en la lista de bloqueados por espera, cambiarle el destino a CLIENTE y derivarlo
}

void app_derivar_pcb(t_pcb* pcb)
{
	//TODO: [APP] Derivar según la posición actual y la destino:
	/*
	 * 1. si al repartidor le toca descansar, se va a la lista de bloqueados por descanso
	 * 2. si posición_actual == posicion_destino y el destino es restaurante, obtiene el pedido y...
	 * 	  a. ...si está listo, se dirige hacia el cliente
	 * 	  b. ...si no está listo, se va a la lista de bloqueados por espera
	 * 3. si posición_actual == posicion_destino y el destino es cliente, finaliza el pedido
	 * 4. si no llegó a su posición destino ni le toca descansar, se va a la lista de ready
	 *
	 * */
}

//********* FUNCIONES PRIVADAS

static void app_asignar_repartidor(t_pcb* pcb)
{
	while(true)
	{
		//Extrae pcb sin asignar
		t_pcb* pcb = queue_sync_pop(pcbs_nuevos, &pcbs_nuevos_mutex, &pcbs_nuevos_sem);

		CS_LOG_DEBUG("Se extrajo el PCB: {CLIENTE: %s} {RESTAURANTE: %s} {POS_DESTINO: [%d,%d]} {ID_PEDIDO: %d}"
				, pcb->cliente
				, pcb->restaurante
				, pcb->posicionRestaurante.x
				, pcb->posicionRestaurante.y
				, pcb->id_pedido
		);

		//Espera a que haya repartidores libres
		sem_wait(&repartidores_libres_sem);

		//Asigna un repartidor más cercano al restaurante (de haber varios, se asigna el más cercano)
		pthread_mutex_lock(&repartidores_libres_mutex);

		bool menor_distancia (t_repartidor* repartidor1, t_repartidor* repartidor2) {
			t_pos vector_distancia1 = calcular_vector_distancia(repartidor1->posicion, pcb->posicionRestaurante);
			t_pos vector_distancia2 = calcular_vector_distancia(repartidor2->posicion, pcb->posicionRestaurante);

			return calcular_norma(vector_distancia1) < calcular_norma(vector_distancia2);
		}
		list_sort(repartidores_libres, (void*) menor_distancia);  //Ordeno la lista por menor distancia
		pcb->repartidor = list_remove(repartidores_libres, 0); //Me quedo con el head de la lista y asi obtengo al repartidor mas cercano disponible

		pthread_mutex_unlock(&repartidores_libres_mutex);

		CS_LOG_DEBUG("Se asignó el repartidor: {ID: %d} {POS_REPARTIDOR: [%d,%d]} {POS_DESTINO: [%d,%d]}"
				, pcb->repartidor->id
				, pcb->repartidor->posicion.x
				, pcb->repartidor->posicion.y
				, pcb->posicionRestaurante.x
				, pcb->posicionRestaurante.y
		);

		//Deriva el pcb a la queue correspondiente (ready/bloqueado/etc)
		app_derivar_pcb(pcb);
	}
}

