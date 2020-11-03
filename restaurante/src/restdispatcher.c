#include "restdispatcher.h"

static int QUANTUM = 0;

static t_list* 	queues_ready;

static rest_ciclo_t**  array_sem_ciclo_cpu;

static t_queue* 	   queue_entrada_salida;
static pthread_mutex_t mutex_entrada_salida;

static t_list*	       lista_blocked;
static pthread_mutex_t mutex_blocked;

static rest_cola_ready_t* rest_cola_ready_create(char* comida);
static rest_cola_ready_t* rest_cola_ready_get(char* comida);
static rest_dispatcher_t* rest_dispatcher_create(rest_cola_ready_t* queue_ready);

static void rest_cocinero_routine(rest_dispatcher_t* self);
static void rest_horno_routine(rest_dispatcher_t* self);
static void rest_reposo_routine(rest_dispatcher_t* self);

static rest_pcb_t* rest_derivar_si_necesario(rest_pcb_t* pcb);

static void rest_pcb_destroy(rest_pcb_t* pcb);

uint32_t rest_dispatcher_init(t_rta_obt_rest* metadata)
{
	//Crea sus estructuras administrativas
	queues_ready = list_create();

	array_sem_ciclo_cpu = (rest_ciclo_t**)string_array_new();

	queue_entrada_salida = queue_create();
	pthread_mutex_init(&mutex_entrada_salida, NULL);

	lista_blocked = list_create();
	pthread_mutex_init(&mutex_blocked, NULL);

	if(!strcmp(cs_config_get_string("ALGORITMO_DE_PLANIFICACION"), "RR")) {
		QUANTUM = cs_config_get_int("QUANTUM");
	}

	//Crea las queues con afinidades
	void _crear_queues(char* comida) {
		rest_cola_ready_t* queue_cocinero = rest_cola_ready_get(comida);
		if(queue_cocinero == NULL)
		{
			queue_cocinero = rest_cola_ready_create(comida);
			list_add(queues_ready, (void*) queue_cocinero);
			CS_LOG_DEBUG("Se creó la cola para: %s", comida);
		}

		pthread_t thread_cocinero;
		pthread_create(
				  &thread_cocinero
				, NULL
				, (void*) rest_cocinero_routine
				, (void*) rest_dispatcher_create(queue_cocinero)
		);
		pthread_detach(thread_cocinero);
	}
	string_iterate_lines(metadata->afinidades, _crear_queues);

	int cant_cocineros_afines = string_array_size(metadata->afinidades);
	if(metadata->cant_cocineros > cant_cocineros_afines)
	{
		//Crea una queue para comidas restantes
		rest_cola_ready_t* queue_restantes = rest_cola_ready_create(NULL);
		CS_LOG_DEBUG("Se creó una cola para comidas restantes");

		void _agregar_platos_restantes(t_comida_menu* comida) {
			if(rest_cola_ready_get(comida->comida) == NULL)
			{
				//TODO: [RESTAURANTE] Los precios sirven de algo ??
				list_add(queue_restantes->comidas, strdup(comida->comida));
				CS_LOG_DEBUG("Se agregó %s a la cola de comidas restantes", comida->comida);
			}
		}
		list_iterate(metadata->menu, (void*) _agregar_platos_restantes);
		list_add(queues_ready, (void*) queue_restantes);

		//Crea los cocineros sin afinidades
		for(int i = cant_cocineros_afines; i < metadata->cant_cocineros; i++)
		{
			pthread_t thread_cocinero;
			pthread_create(
					  &thread_cocinero
					, NULL
					, (void*) rest_cocinero_routine
					, (void*) rest_dispatcher_create(queue_restantes)
			);
			pthread_detach(thread_cocinero);
		}
	}

	//Crea los hornos
	for(int i = 0; i < metadata->cant_hornos; i++)
	{
		pthread_t thread_horno;
		pthread_create(
				  &thread_horno
				, NULL
				, (void*) rest_horno_routine
				, (void*) rest_dispatcher_create(NULL)
		);
		pthread_detach(thread_horno);
	}

	//Crea el hilo de platos en reposo
	pthread_t thread_reposo;
	pthread_create(
			  &thread_reposo
			, NULL
			, (void*) rest_reposo_routine
			, (void*) rest_dispatcher_create(NULL)
	);
	pthread_detach(thread_reposo);

	//Obtiene el resto de la metadata
	uint32_t cant_pedidos = metadata->cant_pedidos;

	mi_posicion.x = metadata->pos_restaurante.x;
	mi_posicion.y = metadata->pos_restaurante.y;
	CS_LOG_DEBUG("Se obtuvo la posición: (%d:%d)", mi_posicion.x, mi_posicion.y);

	cs_msg_destroy(metadata, OPCODE_RESPUESTA_OK, OBTENER_RESTAURANTE);

	return cant_pedidos;
}

void rest_iniciar_ciclo_cpu(void)
{
	CS_LOG_TRACE("Inició el ciclo de ejecución");
	void _hacer_signal_ejecucion(rest_ciclo_t* semaforo) {
		sem_post(&semaforo->inicio_ejecucion);
	}
	string_iterate_lines((char**)array_sem_ciclo_cpu, (void*) _hacer_signal_ejecucion);

	void _hacer_wait_ejecucion(rest_ciclo_t* semaforo) {
		sem_wait(&semaforo->fin_ejecucion);
	}
	string_iterate_lines((char**)array_sem_ciclo_cpu, (void*) _hacer_wait_ejecucion);

	CS_LOG_TRACE("Inició el ciclo de derivación");
	void _hacer_signal_derivacion(rest_ciclo_t* semaforo) {
		sem_post(&semaforo->inicio_derivacion);
	}
	string_iterate_lines((char**)array_sem_ciclo_cpu, (void*) _hacer_signal_derivacion);
}

void rest_esperar_fin_ciclo_cpu(void)
{
	void _hacer_wait_derivacion(rest_ciclo_t* semaforo) {
		sem_wait(&semaforo->fin_derivacion);
	}
	string_iterate_lines((char**)array_sem_ciclo_cpu, (void*) _hacer_wait_derivacion);
	CS_LOG_TRACE("Finalizó el ciclo de derivación");
	CS_LOG_TRACE("Finalizó el ciclo de ejecución");
}

int rest_derivar_pcb(rest_pcb_t* pcb)
{
	t_paso_receta* siguiente_paso = list_get(pcb->pasos_restantes, 0);
	if(siguiente_paso == NULL)
	{
		pcb->estado = ESTADO_EXIT;
		CS_LOG_INFO("El plato está listo: {PID: %d} {ESTADO: %s} {COMIDA: %s} {ID_PEDIDO: %d}"
				, pcb->id
				, rest_estado_to_str(pcb->estado)
				, pcb->comida
				, pcb->pedido_id
		);
		rest_plato_listo(pcb->conexion, pcb->mutex_conexion, pcb->comida, pcb->pedido_id);
		rest_pcb_destroy(pcb);

		return 1;
	}

	if(string_equals_ignore_case(siguiente_paso->paso, "Reposar"))
	{
		pcb->estado = ESTADO_BLOCK;
		CS_LOG_INFO("El plato se dejó en reposo: {PID: %d} {ESTADO: %s} {COMIDA: %s} {ID_PEDIDO: %d}",
				pcb->id, rest_estado_to_str(pcb->estado), pcb->comida, pcb->pedido_id
		);
		pthread_mutex_lock(&mutex_blocked);
		list_add(lista_blocked, (void*) pcb);
		pthread_mutex_unlock(&mutex_blocked);
	} else if(string_equals_ignore_case(siguiente_paso->paso, "Hornear"))
	{
		pcb->estado = ESTADO_BLOCK;
		queue_sync_push(queue_entrada_salida, &mutex_entrada_salida, NULL, (void*) pcb);
	} else
	{
		rest_cola_ready_t* queue = rest_cola_ready_get(pcb->comida);
		if(queue == NULL) {
			rest_pcb_destroy(pcb);
			return -1;
		}
		pcb->estado = ESTADO_READY;
		CS_LOG_DEBUG("El plato está ready: {%s, %d}", pcb->comida, pcb->pedido_id);
		queue_sync_push(queue->queue, &queue->mutex_queue, NULL, (void*) pcb);
	}

	return 0;
}

//Funciones privadas

static rest_cola_ready_t* rest_cola_ready_create(char* comida)
{
	rest_cola_ready_t* ready = malloc(sizeof(rest_cola_ready_t));
	ready->comidas = list_create();
	ready->queue = queue_create();
	pthread_mutex_init(&ready->mutex_queue, NULL);
	if(comida) {
		list_add(ready->comidas, strdup(comida));
	}
	
	return ready;
}

static rest_cola_ready_t* rest_cola_ready_get(char* comida)
{
	bool _find_by_name(rest_cola_ready_t* queue) {
		bool _encontrar_comida(char* afinidad) {
			return !strcmp(afinidad, comida);
		}
		return list_find(queue->comidas, (void*)_encontrar_comida) != NULL;
	}
	return list_find(queues_ready, (void*) _find_by_name);
}

static rest_dispatcher_t* rest_dispatcher_create(rest_cola_ready_t* queue_ready)
{
	rest_dispatcher_t* nuevo = malloc(sizeof(rest_dispatcher_t));

	//Semaforos
	nuevo->sem_ciclo = malloc(sizeof(rest_ciclo_t));

	sem_init(&nuevo->sem_ciclo->inicio_ejecucion, 0, 0);
	sem_init(&nuevo->sem_ciclo->inicio_derivacion, 0, 0);
	sem_init(&nuevo->sem_ciclo->fin_ejecucion, 0, 0);
	sem_init(&nuevo->sem_ciclo->fin_derivacion, 0, 0);

	string_array_push((char***)&array_sem_ciclo_cpu, (char*) nuevo->sem_ciclo);

	nuevo->ready = queue_ready; //Puede ser NULL para la cola de I/O o la de reposo
	nuevo->asignado = NULL;

	return nuevo;
}

static void rest_cocinero_routine(rest_dispatcher_t* self)
{
	CS_LOG_DEBUG("Se creó un cocinero.");
	int ciclos = 0;

	while(true)
	{
		sem_wait(&self->sem_ciclo->inicio_ejecucion);
		if(!self->asignado)
		{
			self->asignado = queue_sync_pop(self->ready->queue, &self->ready->mutex_queue, NULL);
			if(self->asignado)
			{
				self->asignado->estado = ESTADO_EXEC;
				t_paso_receta* siguiente_paso = list_get(self->asignado->pasos_restantes, 0);
				CS_LOG_INFO("Se empezó a %s: {PID: %d} {ESTADO: %s} {COMIDA: %s} {ID_PEDIDO: %d}"
						, siguiente_paso->paso
						, self->asignado->id
						, rest_estado_to_str(self->asignado->estado)
						, self->asignado->comida
						, self->asignado->pedido_id
				);
			}
		}
		if(self->asignado)
		{
			//Obtiene el siguiente paso y ejecuta un ciclo de CPU
			t_paso_receta* siguiente_paso = list_get(self->asignado->pasos_restantes, 0);
			siguiente_paso->tiempo--;
			CS_LOG_DEBUG("Se está cocinando: {PID: %d} {PASO: %s} {TIEMPO_RESTANTE: %d -> %d}",
					self->asignado->id, siguiente_paso->paso, siguiente_paso->tiempo + 1, siguiente_paso->tiempo
			);
		}
		sem_post(&self->sem_ciclo->fin_ejecucion);

		sem_wait(&self->sem_ciclo->inicio_derivacion);
		if(self->asignado)
		{
			//Si terminó ese paso, se terminó la ejecución de ese plato
			self->asignado = rest_derivar_si_necesario(self->asignado);

			//Si hay fin de quantum y hay pcbs en READY, se desaloja
			if( self->asignado != NULL
				&& ++ciclos == QUANTUM
				&& queue_sync_has_elements(self->ready->queue, &self->ready->mutex_queue) )
			{
				CS_LOG_INFO("Fin de QUANTUM para: {PID: %d} {ESTADO: %s} {COMIDA: %s} {ID_PEDIDO: %d}"
						, self->asignado->id
						, rest_estado_to_str(self->asignado->estado)
						, self->asignado->comida
						, self->asignado->pedido_id
				);
				rest_derivar_pcb(self->asignado);
				self->asignado = NULL;
			}
		}
		sem_post(&self->sem_ciclo->fin_derivacion);

		if(!self->asignado) ciclos = 0;
	}
}

static void rest_horno_routine(rest_dispatcher_t* self)
{
	CS_LOG_DEBUG("Se creó un horno.");

	while(true)
	{
		sem_wait(&self->sem_ciclo->inicio_ejecucion);
		if(!self->asignado)
		{
			self->asignado = queue_sync_pop(queue_entrada_salida, &mutex_entrada_salida, NULL);
			if(self->asignado)
			{
				CS_LOG_INFO("El plato entró al horno: {PID: %d} {ESTADO: %s} {COMIDA: %s} {ID_PEDIDO: %d}"
						, self->asignado->id
						, rest_estado_to_str(self->asignado->estado)
						, self->asignado->comida
						, self->asignado->pedido_id
				);
			}
		}
		if(self->asignado)
		{
			//Obtiene el siguiente paso y ejecuta un ciclo de CPU
			t_paso_receta* siguiente_paso = list_get(self->asignado->pasos_restantes, 0);
			siguiente_paso->tiempo--;
			CS_LOG_DEBUG("Se está horneando: {PID: %d} {PASO: %s} {TIEMPO_RESTANTE: %d -> %d}",
					self->asignado->id, siguiente_paso->paso, siguiente_paso->tiempo + 1, siguiente_paso->tiempo
			);
		}
		sem_post(&self->sem_ciclo->fin_ejecucion);

		sem_wait(&self->sem_ciclo->inicio_derivacion);
		if(self->asignado)
		{
			//Si terminó ese paso, se terminó la ejecución de ese plato
			self->asignado = rest_derivar_si_necesario(self->asignado);
		}
		sem_post(&self->sem_ciclo->fin_derivacion);
	}
}

static void rest_reposo_routine(rest_dispatcher_t* self)
{
	CS_LOG_DEBUG("Se creó el thread reposo.");

	while(true)
	{
		sem_wait(&self->sem_ciclo->inicio_ejecucion);
		//Ejecuta un ciclo para todos los platos que estén en reposo
		void _ejecutar_ciclo(rest_pcb_t* elemento) {
			t_paso_receta* siguiente_paso = list_get(elemento->pasos_restantes, 0);
			siguiente_paso->tiempo--;
			CS_LOG_DEBUG("Se está reposando: {PID: %d} {PASO: %s} {TIEMPO_RESTANTE: %d -> %d}",
					elemento->id, siguiente_paso->paso, siguiente_paso->tiempo + 1, siguiente_paso->tiempo
			);
		}
		pthread_mutex_lock(&mutex_blocked);
		list_iterate(lista_blocked, (void*) _ejecutar_ciclo);
		pthread_mutex_unlock(&mutex_blocked);
		sem_post(&self->sem_ciclo->fin_ejecucion);

		sem_wait(&self->sem_ciclo->inicio_derivacion);
		//Deriva todos los platos que hayan concluido su reposo, y los quita de la lista
		int i = 0;
		void _derivar_si_necesario(rest_pcb_t* elemento) {
			if(rest_derivar_si_necesario(elemento) == NULL) {
				list_remove(lista_blocked, i);
			} else {
				i++;
			}
		}
		pthread_mutex_lock(&mutex_blocked);
		list_iterate(lista_blocked, (void*) _derivar_si_necesario);
		pthread_mutex_unlock(&mutex_blocked);
		sem_post(&self->sem_ciclo->fin_derivacion);
	}
}

static rest_pcb_t* rest_derivar_si_necesario(rest_pcb_t* pcb)
{
	//Si el paso terminó, lo quita de la cima y deriva el plato
	t_paso_receta* siguiente_paso = list_get(pcb->pasos_restantes, 0);
	if(siguiente_paso->tiempo == 0)
	{
		CS_LOG_INFO("Se terminó de %s: {PID: %d} {ESTADO: %s} {COMIDA: %s} {ID_PEDIDO: %d}"
				, siguiente_paso->paso
				, pcb->id
				, rest_estado_to_str(pcb->estado)
				, pcb->comida, pcb->pedido_id
		);

		void _destruir_paso(t_paso_receta* paso) {
			free(paso->paso);
			free(paso);
		}
		list_remove_and_destroy_element(pcb->pasos_restantes, 0, (void*) _destruir_paso);
		siguiente_paso = list_get(pcb->pasos_restantes, 0);

		//Si está en EXEC y el siguiente paso no implica irse a BLOCK, continúa
		if(siguiente_paso && pcb->estado == ESTADO_EXEC
				&& strcmp(siguiente_paso->paso, "Reposar")
				&& strcmp(siguiente_paso->paso, "Hornear") )
		{
			CS_LOG_INFO("Se continuará con el paso %s: {PID: %d} {ESTADO: %s} {COMIDA: %s} {ID_PEDIDO: %d}"
					, siguiente_paso->paso
					, pcb->id
					, rest_estado_to_str(pcb->estado)
					, pcb->comida
					, pcb->pedido_id
			);
		} else
		{
			rest_derivar_pcb(pcb);
			pcb = NULL;
		}
	} else
	{
		CS_LOG_TRACE("No es necesario derivar: {%s,%d}", pcb->comida, pcb->pedido_id);
	}

	return pcb;
}

static void rest_pcb_destroy(rest_pcb_t* pcb)
{
	free(pcb->comida);
	list_destroy(pcb->pasos_restantes);
	free(pcb);
}




