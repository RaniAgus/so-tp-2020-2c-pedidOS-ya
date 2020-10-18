#include "restplanificador.h"

static uint32_t        id_pedido;
static pthread_mutex_t mutex_id_pedido;

void rest_planificador_init(t_rta_obt_rest* metadata)
{
	id_pedido = metadata->cant_pedidos;
	pthread_mutex_init(&mutex_id_pedido, NULL);
	mi_posicion.x = metadata->pos_restaurante.x;
	mi_posicion.y = metadata->pos_restaurante.y;

	//TODO: Guardar el resto de la metadata del Restaurante

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
		void _get_conn(rest_cliente_t* encontrado) {
			pcb_nuevo->conexion = encontrado->conexion;
			pcb_nuevo->mutex_conexion = encontrado->mutex_conexion;
		}
		rest_cliente_get(cliente, _get_conn);
	}
	else
	{
		pcb_nuevo->mutex_conexion = NULL;
		pcb_nuevo->conexion = -1;
	}

	//TODO: Agregar PCB
	CS_LOG_TRACE("Se va a agregar el PCB a la cola de planificación");
}

