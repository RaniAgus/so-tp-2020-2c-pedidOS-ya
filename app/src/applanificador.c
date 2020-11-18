#include "applanificador.h"

static pthread_t thread_planificador;

static t_queue*			pcbs_nuevos;
static pthread_mutex_t 	pcbs_nuevos_mutex;
static sem_t 			pcbs_nuevos_sem;

static void app_asignar_repartidor(t_pcb* pcb);
static t_repartidor* app_obtener_repartidor_libre(t_pos destino);

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
    queue_sync_push(pcbs_nuevos, &pcbs_nuevos_mutex, &pcbs_nuevos_sem, pcb);
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

		//Extrae repartidor libre
		t_repartidor* repartidor = app_obtener_repartidor_libre(pcb->posicionRestaurante);

		//Asigna el PCB al repartidor
		repartidor->pcb = pcb;

		CS_LOG_DEBUG("Se asignó el repartidor: {ID: %d} {POS_REPARTIDOR: [%d,%d]} {POS_DESTINO: [%d,%d]}"
				, repartidor->id
				, repartidor->posicion.x
				, repartidor->posicion.y
				, repartidor->pcb->posicionRestaurante.x
				, repartidor->pcb->posicionRestaurante.y
		);

		//Deriva el repartidor a la queue correspondiente (ready/bloqueado/etc)
		app_derivar_repartidor(repartidor);
	}
}

static t_repartidor* app_obtener_repartidor_libre(t_pos destino)
{
	t_repartidor* repartidor;

	//Espera a que haya repartidores libres
	sem_wait(&repartidores_libres_sem);

	//Espera si hay otro hilo manipulando la lista
	pthread_mutex_lock(&repartidores_libres_mutex);

	//Busca al repartidor más cercano al restaurante
	bool menor_distancia (t_repartidor* repartidor1, t_repartidor* repartidor2) {
		t_pos vector_distancia1 = calcular_vector_distancia(repartidor1->posicion, destino);
		t_pos vector_distancia2 = calcular_vector_distancia(repartidor2->posicion, destino);

		return calcular_norma(vector_distancia1) < calcular_norma(vector_distancia2);
	}
	list_sort(repartidores_libres, (void*) menor_distancia);  //Ordeno la lista por menor distancia
	repartidor = list_remove(repartidores_libres, 0); //Me quedo con el head de la lista y asi obtengo al repartidor mas cercano disponible

	//Libera el mutex
	pthread_mutex_unlock(&repartidores_libres_mutex);

	return repartidor;
}

