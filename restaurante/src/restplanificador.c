#include "restplanificador.h"

static uint32_t        id_pedido;
static pthread_mutex_t mutex_id_pedido;

static int				plato_control_block_id;
static pthread_mutex_t	mutex_plato_control_block_id;

static int rest_generar_pcb_id(void);

void rest_planificador_init(uint32_t pedidos)
{
	id_pedido = pedidos;
	pthread_mutex_init(&mutex_id_pedido, NULL);
	CS_LOG_DEBUG("Se obtuvo el id inicial: %d", pedidos);

	plato_control_block_id = 0;
	pthread_mutex_init(&mutex_plato_control_block_id, NULL);
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
	rest_pcb_t* pcb_nuevo = malloc(sizeof(rest_pcb_t));
	pcb_nuevo->id = rest_generar_pcb_id();
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

	CS_LOG_INFO("Se creó un nuevo plato: {PID: %d} {ESTADO: %s} {COMIDA: %s} {ID_PEDIDO: %d}",
			pcb_nuevo->id, rest_estado_to_str(pcb_nuevo->estado), pcb_nuevo->comida, pcb_nuevo->pedido_id
	);

	rest_derivar_pcb(pcb_nuevo);

	return 0;
}

static int rest_generar_pcb_id(void)
{
	pthread_mutex_lock(&mutex_plato_control_block_id);
	int generado = ++plato_control_block_id;
	pthread_mutex_unlock(&mutex_plato_control_block_id);

	return generado;
}
