#include "appqueues.h"

typedef enum { FIFO, HRRN, SJFSD } e_algoritmo;

static e_algoritmo ALGORITMO_PLANIFICACION;
static double      ALPHA;

static t_list* 		   repartidores_libres;
static pthread_mutex_t repartidores_libres_mutex;
static sem_t		   repartidores_libres_sem;

static t_list* 			ready_queue;
static pthread_mutex_t	ready_mutex;
//TODO: [APP] Ver si hay que agregar semáforo a la cola de ready

//TODO: [APP] Crear e iniciar las listas de bloqueado por descanso y bloqueado por espera

static e_algoritmo app_obtener_algoritmo(void);

void app_iniciar_colas_planificacion(void)
{
	ALGORITMO_PLANIFICACION = app_obtener_algoritmo();
	ALPHA = cs_config_get_double("ALPHA");

	repartidores_libres = list_create();
	pthread_mutex_init(&repartidores_libres_mutex, NULL);
	sem_init(&repartidores_libres_sem, 0, 0);

	ready_queue = list_create();
	pthread_mutex_init(&ready_mutex, NULL);
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

void app_derivar_repartidor(t_repartidor* repartidor)
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

/********************************** REPARTIDORES LIBRES **********************************/

void app_agregar_repartidor_libre(t_repartidor* repartidor)
{
	pthread_mutex_lock(&repartidores_libres_mutex);
	list_add(repartidores_libres, repartidor);
	pthread_mutex_unlock(&repartidores_libres_mutex);

	CS_LOG_DEBUG("El repartidor está disponible: {ID: %d} {POS_REPARTIDOR: [%d,%d]}"
			, repartidor->id
			, repartidor->posicion.x
			, repartidor->posicion.y
	);

	sem_post(&repartidores_libres_sem);
}

t_repartidor* app_obtener_repartidor_libre(t_pos destino)
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

/**************************************** READY ****************************************/

static void app_ordenar_ready(void);
static double proxima_rafaga(t_pcb* pcb);
static double response_ratio(t_pcb* pcb);

t_repartidor* app_ready_pop(void)
{
	t_repartidor* repartidor;

	//Espera a que no haya otro hilo usando la cola de ready
	pthread_mutex_lock(&ready_mutex);

	//Ordena la cola de ready según prioridad y quita el primer elemento
	app_ordenar_ready();
	repartidor = list_remove(ready_queue, 0);

	//Actualiza la espera de los demás repartidores (sirve para HRRN)
	void actualizar_espera(t_repartidor* pcb) {
		repartidor->pcb->espera++;
	}
	list_iterate(ready_queue, (void*)actualizar_espera);

	//Libera el bloqueo para que otros hilos usen la cola de ready
	pthread_mutex_unlock(&ready_mutex);

	//Actualiza la información del repartidor extraído (sirve para SJF y HRRN)
	if(repartidor != NULL) {
		repartidor->pcb->estimacion_anterior = proxima_rafaga(repartidor->pcb);
		repartidor->pcb->ultima_rafaga = 0;
		repartidor->pcb->espera = 0;
	}

	return repartidor;
}

void app_ready_push(t_pcb* pcb)
{
	pthread_mutex_lock(&ready_mutex);
	list_add(ready_queue, pcb);
	pthread_mutex_unlock(&ready_mutex);
}

/********************************** FUNCIONES PRIVADAS **********************************/

static e_algoritmo app_obtener_algoritmo(void)
{
	const char* algoritmos_str[] = {"FIFO", "HRRN", "SJF-SD", NULL};

	const char* e_algoritmo_to_str(int value) {
		return algoritmos_str[value];
	}

	return cs_string_to_enum(cs_config_get_string("ALGORITMO_PLANIFICACION"), e_algoritmo_to_str);
}

static void app_ordenar_ready(void)
{
	bool mayor_prioridad(t_repartidor* repartidor1, t_repartidor* repartidor2) {
		switch(ALGORITMO_PLANIFICACION)
		{
		case SJFSD: //SJF ordena según la estimación menor
			return proxima_rafaga(repartidor1->pcb) <= proxima_rafaga(repartidor2->pcb);
		case HRRN: //HRRN ordena según el response ratio mayor
			return response_ratio(repartidor1->pcb) >= response_ratio(repartidor2->pcb);
		default: //FIFO no reordena nada
			return true;
		}
	}
	list_sort(ready_queue, (void*)mayor_prioridad);
}

static double proxima_rafaga(t_pcb* pcb) {
	return pcb->ultima_rafaga * ALPHA + pcb->estimacion_anterior * (1 - ALPHA);
}

static double response_ratio(t_pcb* pcb) {
	return 1 + pcb->espera / proxima_rafaga(pcb);
}
