#include "restplaniflargoplazo.h"

static uint32_t        id_pedido;
static pthread_mutex_t mutex_id_pedido;

static t_list* 	queues_ready;

static rest_cola_ready_t* rest_cola_ready_create(char* comida);
static rest_cola_ready_t* rest_cola_ready_get(char* comida);

void rest_planificador_largo_plazo_init(t_rta_obt_rest* metadata)
{
	queues_ready = list_create();

	//Crea las queues con afinidades
	void _crear_queues(char* comida) {
		rest_cola_ready_t* queue_cocinero = rest_cola_ready_get(comida);
		if(queue_cocinero == NULL)
		{
			queue_cocinero = rest_cola_ready_create(comida);
			list_add(queues_ready, (void*) queue_cocinero);
			CS_LOG_TRACE("Se creó la cola para: %s", comida);
		}

		pthread_t thread_cocinero;
		PTHREAD_CREATE(&thread_cocinero, rest_cocinero_routine, queue_cocinero);
		pthread_detach(thread_cocinero);
	}
	string_iterate_lines(metadata->afinidades, _crear_queues);

	int cant_cocineros_afines = string_array_size(metadata->afinidades);
	if(metadata->cant_cocineros > cant_cocineros_afines)
	{
		//Crea una queue para comidas restantes
		rest_cola_ready_t* queue_restantes = rest_cola_ready_create(NULL);
		CS_LOG_TRACE("Se creó una cola para comidas restantes");

		void _agregar_platos_restantes(t_comida_menu* comida) {
			if(rest_cola_ready_get(comida->comida) == NULL)
			{
				//TODO: [RESTAURANTE] Los precios sirven de algo ??
				list_add(queue_restantes->comidas, strdup(comida->comida));
				CS_LOG_TRACE("Se agregó %s a la cola de comidas restantes", comida->comida);
			}
		}
		list_iterate(metadata->menu, (void*) _agregar_platos_restantes);
		list_add(queues_ready, (void*) queue_restantes->comidas);

		for(int i = cant_cocineros_afines; i < metadata->cant_cocineros; i++)
		{
			pthread_t thread_cocinero;
			PTHREAD_CREATE(&thread_cocinero, rest_cocinero_routine, queue_restantes);
			pthread_detach(thread_cocinero);
		}
	}

	for(int i = 0; i < metadata->cant_hornos; i++)
	{
		//TODO: [RESTAURANTE] Ver parámetro que pueda recibir horno para inicializarse
		pthread_t thread_horno;
		PTHREAD_CREATE(&thread_horno, rest_cocinero_routine, NULL);
		pthread_detach(thread_horno);
	}

	id_pedido = metadata->cant_pedidos;
	pthread_mutex_init(&mutex_id_pedido, NULL);
	mi_posicion.x = metadata->pos_restaurante.x;
	mi_posicion.y = metadata->pos_restaurante.y;

	cs_msg_destroy(metadata, OPCODE_RESPUESTA_OK, OBTENER_RESTAURANTE);
}

uint32_t rest_generar_id(void)
{
	pthread_mutex_lock(&mutex_id_pedido);
	uint32_t generado = ++id_pedido;
	pthread_mutex_unlock(&mutex_id_pedido);

	return generado;
}

int rest_planificar_plato(char* comida, uint32_t pedido_id, t_list* pasos_receta, char* cliente)
{
	rest_cola_ready_t* queue = rest_cola_ready_get(comida);
	if(queue == NULL) {
		CS_LOG_ERROR("No se pudo crear PCB de la comida: %s", comida);
		return -1;
	}

	rest_pcb_t* pcb_nuevo = malloc(sizeof(rest_pcb_t));
	pcb_nuevo->estado = ESTADO_NEW;

	pcb_nuevo->comida = strdup(comida);
	pcb_nuevo->pedido_id = pedido_id;
	pcb_nuevo->pasos_restantes = cs_receta_duplicate(pasos_receta);
	if(cliente)
	{
		CS_LOG_TRACE("El plato proviene de un Cliente.");
		void _get_conn(rest_cliente_t* encontrado) {
			pcb_nuevo->conexion = encontrado->conexion;
			pcb_nuevo->mutex_conexion = encontrado->mutex_conexion;
		}
		rest_cliente_get(cliente, _get_conn);
	}
	else
	{
		CS_LOG_TRACE("El plato proviene de la App.");
		pcb_nuevo->mutex_conexion = NULL;
		pcb_nuevo->conexion = -1;
	}

	rest_cola_ready_push(queue, pcb_nuevo);
	CS_LOG_TRACE("Se agregó el PCB a la cola de planificación: {%s,%d}", pcb_nuevo->comida, pcb_nuevo->pedido_id);

	return 0;
}

void rest_cola_ready_push(rest_cola_ready_t* queue, rest_pcb_t* pcb)
{
	queue_sync_push(queue->queue, &queue->mutex_queue, NULL, (void*) pcb);
}

rest_pcb_t* rest_cola_ready_pop(rest_cola_ready_t* queue)
{
	return queue_sync_pop(queue->queue, &queue->mutex_queue, NULL);
}

//Funciones privadas

static rest_cola_ready_t* rest_cola_ready_create(char* comida)
{
	rest_cola_ready_t* ready = malloc(sizeof(rest_cola_ready_t));
	ready->comidas = list_create();
	ready->queue = queue_create();
	pthread_mutex_init(&ready->mutex_queue, NULL);

	if(comida) list_add(ready->comidas, comida);

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
