#include "restplanificador.h"

static t_list* queues_ready;

static uint32_t        id_pedido;
static pthread_mutex_t mutex_id_pedido;

static rest_cola_ready_t* rest_cola_ready_create(char* comida);
static rest_cola_ready_t* rest_cola_ready_find(char* comida);

void rest_planificador_init(t_rta_obt_rest* metadata)
{
	//Crea las queues con afinidades
	void _crear_queues(char* comida) {
		rest_cola_ready_t* queue_cocinero = rest_cola_ready_find(comida);
		if(queue_cocinero == NULL) {
			queue_cocinero = rest_cola_ready_create(comida);
			list_add(queues_ready, (void*) queue_cocinero);
		}

		//TODO: Crear hilos de cocineros acá
	}
	string_iterate_lines(metadata->afinidades, _crear_queues);

	//Si hay más cocineros que afinidades, crea la queue de platos restantes
	if(metadata->cant_cocineros > string_array_size(metadata->afinidades))
	{
		rest_cola_ready_t* cola_restantes = rest_cola_ready_create(NULL);

		void _agregar_platos_restantes(t_comida_menu* comida) {
			if(rest_cola_ready_find(comida->comida) == NULL) {
				list_add(cola_restantes->comidas, strdup(comida->comida));
				//TODO: Incluir precios ??
			}
		}
		list_iterate(metadata->menu, (void*) _agregar_platos_restantes);
		list_add(queues_ready, (void*) cola_restantes->comidas);
	}

	//TODO: Crear hilos de hornos acá ??

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

void rest_planificar_plato(char* comida, uint32_t pedido_id, t_list* pasos_receta, char* cliente)
{
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

	//TODO: Agregar PCB
	CS_LOG_TRACE("Se va a agregar el PCB a la cola de planificación");
}

static rest_cola_ready_t* rest_cola_ready_create(char* comida)
{
	rest_cola_ready_t* ready = malloc(sizeof(rest_cola_ready_t));
	ready->comidas = list_create();
	ready->queue = queue_create();
	pthread_mutex_init(&ready->mutex_queue, NULL);
	sem_init(&ready->sem_queue, 0, 0);

	if(comida) list_add(ready->comidas, comida);

	return ready;
}

static rest_cola_ready_t* rest_cola_ready_find(char* comida)
{
	bool _find_by_name(rest_cola_ready_t* queue) {
		bool _encontrar_comida(char* afinidad) {
			return !strcmp(afinidad, comida);
		}
		return list_find(queue->comidas, (void*)_encontrar_comida) != NULL;
	}
	return list_find(queues_ready, (void*) _find_by_name);
}
