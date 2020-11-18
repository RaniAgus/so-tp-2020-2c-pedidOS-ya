#include "apprepartidores.h"

typedef enum { FIFO, HRRN, SJFSD } e_algoritmo;

static e_algoritmo ALGORITMO_PLANIFICACION;
static double      ALPHA;

static t_list* 			ready_queue;
static pthread_mutex_t	ready_mutex;
//TODO: [APP] Ver si hay que agregar semáforo a la cola de ready

//TODO: [APP] Crear e iniciar las listas de bloqueado por descanso y bloqueado por espera

static e_algoritmo app_obtener_algoritmo(void);
static void app_agregar_repartidor_libre(t_repartidor* repartidor);
static void app_ready_push(t_pcb* pcb);
static t_repartidor* app_ready_pop(void);

void app_inicializar_repartidores(void)
{
	ALGORITMO_PLANIFICACION = app_obtener_algoritmo();
	ALPHA = cs_config_get_double("ALPHA");

	ready_queue = list_create();
	pthread_mutex_init(&ready_mutex, NULL);



	char** paresDeCoordenadas = cs_config_get_array_value("REPARTIDORES");
	char** frecuenciasDeDescanso = cs_config_get_array_value("FRECUENCIA_DE_DESCANSO");
	char** tiemposDeDescanso = cs_config_get_array_value("TIEMPO_DE_DESCANSO");

	for(int i = 0; paresDeCoordenadas[i] != NULL; i++)
	{
		char** posicion = string_split(paresDeCoordenadas[i], "|");
		t_repartidor* repartidor = malloc(sizeof(t_repartidor));

		repartidor->id = i+1;
		repartidor->posicion.x = atoi(posicion[0]);
		repartidor->posicion.y = atoi(posicion[1]);
		repartidor->frecuencia_de_descanso = atoi(frecuenciasDeDescanso[i]);
		repartidor->tiempo_de_descanso = atoi(tiemposDeDescanso[i]);
		repartidor->estimacion_anterior = cs_config_get_double("ESTIMACION_INICIAL");
		repartidor->ultima_rafaga = cs_config_get_double("ESTIMACION_INICIAL");
		repartidor->espera = 0;

		//Al inicio se agregan todos los repartidores creados a la lista de libres
		app_agregar_repartidor_libre(repartidor);

		string_iterate_lines(posicion, (void*) free);
		free(posicion);
	}

	string_iterate_lines(paresDeCoordenadas, (void*) free);
	string_iterate_lines(frecuenciasDeDescanso, (void*) free);
	string_iterate_lines(tiemposDeDescanso, (void*) free);
	free(paresDeCoordenadas);
	free(frecuenciasDeDescanso);
	free(tiemposDeDescanso);
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


/********************************** FUNCIONES PRIVADAS **********************************/

static e_algoritmo app_obtener_algoritmo(void)
{
	const char* algoritmos_str[] = {"FIFO", "HRRN", "SJF-SD", NULL};

	const char* e_algoritmo_to_str(int value) {
		return algoritmos_str[value];
	}

	return cs_string_to_enum(cs_config_get_string("ALGORITMO_PLANIFICACION"), e_algoritmo_to_str);
}

static void app_agregar_repartidor_libre(t_repartidor* repartidor)
{
	pthread_mutex_lock(&repartidores_libres_mutex);
	list_add(repartidores_libres, repartidor);
	pthread_mutex_unlock(&repartidores_libres_mutex);

	sem_post(&repartidores_libres_sem);
}

/**************************************** READY ****************************************/

static void app_ready_push(t_pcb* pcb)
{
	pthread_mutex_lock(&ready_mutex);
	list_add(ready_queue, pcb);
	pthread_mutex_unlock(&ready_mutex);
}

static double proxima_rafaga(t_repartidor* repartidor) {
	return repartidor->ultima_rafaga * ALPHA + repartidor->estimacion_anterior * (1 - ALPHA);
}

static double response_ratio(t_repartidor* repartidor) {
	return 1 + repartidor->espera / proxima_rafaga(repartidor);
}

static t_repartidor* app_ready_pop(void)
{
	t_repartidor* repartidor;

	//Espera a que no haya otro hilo usando la cola de ready
	pthread_mutex_lock(&ready_mutex);

	//Ordena la cola de ready según prioridad
	bool mayor_prioridad(t_repartidor* repartidor1, t_repartidor* repartidor2) {
		switch(ALGORITMO_PLANIFICACION)
		{
		case SJFSD: //SJF ordena según la estimación menor
			return proxima_rafaga(repartidor1) <= proxima_rafaga(repartidor2);
		case HRRN: //HRRN ordena según el response ratio mayor
			return response_ratio(repartidor1) >= response_ratio(repartidor2);
		default: //FIFO no reordena nada
			return true;
		}
	}
	list_sort(ready_queue, (void*)mayor_prioridad);

	//Quita el primer elemento de la lista
	repartidor = list_remove(ready_queue, 0);

	//Actualiza la espera de los demás repartidores (sirve para HRRN)
	void actualizar_espera(t_repartidor* pcb) {
		repartidor->espera++;
	}
	list_iterate(ready_queue, (void*)actualizar_espera);

	//Libera el bloqueo para que otros hilos usen la cola de ready
	pthread_mutex_unlock(&ready_mutex);

	//Actualiza la información del repartidor extraído (sirve para SJF y HRRN)
	if(repartidor != NULL) {
		repartidor->estimacion_anterior = proxima_rafaga(repartidor);
		repartidor->ultima_rafaga = 0;
		repartidor->espera = 0;
	}

	return repartidor;
}


