#include "appqueues.h"

void app_iniciar_colas_planificacion(void)
{
	ALGORITMO_PLANIFICACION = app_obtener_algoritmo();
	ALPHA = cs_config_get_double("ALPHA");

	repartidores_libres = list_create();
	pthread_mutex_init(&repartidores_libres_mutex, NULL);
	sem_init(&repartidores_libres_sem, 0, 0);

	ready_queue = list_create();
	pthread_mutex_init(&ready_mutex, NULL);

	repartidores_esperando = list_create();
	pthread_mutex_init(&repartidores_esperando_mutex, NULL);
}

void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id)
{
	t_repartidor* repartidor;

	//Busca al repartidor según el pedido
    pthread_mutex_lock(&repartidores_esperando_mutex);
    bool encontrar_pedido(t_repartidor* repartidor) {
    	return !strcmp(repartidor->pcb->restaurante, restaurante) && repartidor->pcb->id_pedido == pedido_id;
    }
    repartidor = list_remove_by_condition(repartidores_esperando, (void*)encontrar_pedido);
    pthread_mutex_unlock(&repartidores_esperando_mutex);

    //Si lo encuentra, le cambia el destino a CLIENTE y lo deriva a la queue que corresponda
    if(repartidor != NULL)
    {
    	repartidor->destino = DESTINO_CLIENTE;
    	CS_LOG_DEBUG("El repartidor se moverá hacia el cliente: {ID: %d} {RESTAURANTE: %s} {ID_PEDIDO: %d}"
    			, repartidor->id
				, restaurante
				, pedido_id
		);
    	app_derivar_repartidor(repartidor);
    } else
    {
    	CS_LOG_DEBUG("El repartidor no ha llegado aún al Restaurante: {RESTAURANTE: %s} {ID_PEDIDO: %d}"
    			, restaurante
				, pedido_id
		);
    }
}


void app_liberar_repartidor(t_repartidor* repartidor)
{
	repartidor->pcb = NULL;
	repartidor->destino = 0;

	pthread_mutex_lock(&repartidores_libres_mutex);
	list_add(repartidores_libres, repartidor);
	pthread_mutex_unlock(&repartidores_libres_mutex);
}

void app_derivar_repartidor(t_repartidor* repartidor)
{
	if(repartidor_llego_a_destino(repartidor))
	{
		if(repartidor->destino == DESTINO_RESTAURANTE)
		{
			//Obtiene el pedido
			int8_t resultado;
			t_rta_obt_ped* pedido;

			pedido = app_obtener_pedido(repartidor->pcb->restaurante, repartidor->pcb->id_pedido, &resultado);

			if (pedido->estado_pedido == OPCODE_RESPUESTA_OK) {
				repartidor->destino = DESTINO_CLIENTE;
				app_derivar_repartidor(repartidor);
			}
			else {
				//Se va a la lista de bloqueados por espera
				app_agregar_repartidor_esperando(repartidor);
			}
		} else
		{
			//TODO: revisar que hacer con lo que devuelve app_finalizar_pedido, de momento no hago nada
			app_finalizar_pedido(repartidor->pcb->restaurante, repartidor->pcb->id_pedido, repartidor->pcb->cliente);
			//Libera al repartidor
			app_liberar_repartidor(repartidor);
		}
	} else
	{
		//Lo agrega a la lista de ready
		pthread_mutex_lock(&ready_mutex);
		list_add(ready_queue, repartidor);
		pthread_mutex_unlock(&ready_mutex);
	}
}

bool repartidor_llego_a_destino(t_repartidor* repartidor)
{
	t_pos destino = app_destino_repartidor(repartidor);
	return repartidor->posicion.x == destino.x && repartidor->posicion.y == destino.y;
}

t_pos app_destino_repartidor(t_repartidor* repartidor)
{
	return repartidor->destino == DESTINO_RESTAURANTE ?
		repartidor->pcb->posicionRestaurante : repartidor->pcb->posicionCliente;
}

/********************************** REPARTIDORES BLOQUEADOS POR DESCANSO **********************************/

bool toca_descansar(t_repartidor* repartidor)
{
	return (repartidor->ciclos_sin_descansar == repartidor->frecuencia_de_descanso);
}

void app_agregar_repartidor_descansando(t_repartidor* repartidor)
{
	bool encontrar_repartidor(t_repartidor* elemento) {return elemento->id == repartidor->id;}

	pthread_mutex_lock(&ready_mutex);
	list_remove_by_condition(ready_queue, (void*) encontrar_repartidor);
	pthread_mutex_unlock(&ready_mutex);

	pthread_mutex_lock(&repartidores_descansando_mutex);
	list_add(repartidores_descansando, repartidor);
	pthread_mutex_unlock(&repartidores_descansando_mutex);

	CS_LOG_DEBUG("El repartidor está bloqueado por descanso: {ID: %d} {POS_REPARTIDOR: [%d,%d]}"
			, repartidor->id
			, repartidor->posicion.x
			, repartidor->posicion.y
	);
}

void descansa(t_repartidor* repartidor)
{
	repartidor->ciclos_sin_descansar --;
}

void app_reviso_repartidores_descansados()
{
	pthread_mutex_lock(&repartidores_descansando_mutex);

	int i = 0;

	void ya_descanso(t_repartidor* repartidor) {
		if (repartidor->ciclos_sin_descansar == 0) {
			list_remove(repartidores_descansando, i);
			app_derivar_repartidor(repartidor);
		}
		else i ++;
	}

	list_iterate(repartidores_descansando, (void*) ya_descanso);
	pthread_mutex_unlock(&repartidores_descansando_mutex);

}

/********************************** REPARTIDORES BLOQUEADOS POR ESPERA **********************************/

void app_agregar_repartidor_esperando(t_repartidor* repartidor)
{
	bool encontrar_repartidor(t_repartidor* elemento){return elemento->id == repartidor->id;}

	pthread_mutex_lock(&ready_mutex);
	list_remove_by_condition(ready_queue, (void*) encontrar_repartidor);
	pthread_mutex_unlock(&ready_mutex);

	pthread_mutex_lock(&repartidores_esperando_mutex);
	list_add(repartidores_esperando, repartidor);
	pthread_mutex_unlock(&repartidores_esperando_mutex);

	CS_LOG_DEBUG("El repartidor está bloqueado por espera: {ID: %d} {POS_REPARTIDOR: [%d,%d]}"
			, repartidor->id
			, repartidor->posicion.x
			, repartidor->posicion.y
	);
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

/********************************** FUNCIONES PRIVADAS **********************************/

e_algoritmo app_obtener_algoritmo(void)
{
	const char* algoritmos_str[] = {"FIFO", "HRRN", "SJF-SD", NULL};

	const char* e_algoritmo_to_str(int value) {
		return algoritmos_str[value];
	}

	return cs_string_to_enum(cs_config_get_string("ALGORITMO_PLANIFICACION"), e_algoritmo_to_str);
}

void app_ordenar_ready(void)
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

double proxima_rafaga(t_pcb* pcb) {
	return pcb->ultima_rafaga * ALPHA + pcb->estimacion_anterior * (1 - ALPHA);
}

double response_ratio(t_pcb* pcb) {
	return 1 + pcb->espera / proxima_rafaga(pcb);
}
