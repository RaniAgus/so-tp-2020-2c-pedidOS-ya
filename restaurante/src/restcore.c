#include "restcore.h"

static t_list* 		   lista_clientes;
static pthread_mutex_t mutex_clientes;

static uint32_t        id_pedido;
static pthread_mutex_t mutex_id_pedido;

void rest_core_init(t_rta_obt_rest* metadata)
{
	id_pedido = metadata->cant_pedidos;
	pthread_mutex_init(&mutex_id_pedido, NULL);
	mi_posicion.x = metadata->pos_restaurante.x;
	mi_posicion.y = metadata->pos_restaurante.y;

	//TODO: Guardar el resto de la metadata del Restaurante

	//Crea la lista de clientes
	lista_clientes = list_create();
	pthread_mutex_init(&mutex_clientes, NULL);

	cs_msg_destroy(metadata, OPCODE_RESPUESTA_OK, OBTENER_RESTAURANTE);
}

uint32_t rest_generar_id(void)
{
	pthread_mutex_lock(&mutex_id_pedido);
	uint32_t generado = ++id_pedido;
	pthread_mutex_unlock(&mutex_id_pedido);

	return generado;
}

rest_cliente_t* rest_cliente_create(char* nombre, t_sfd conexion)
{
	rest_cliente_t* cliente = malloc(sizeof(rest_cliente_t));

	cliente->nombre = strdup(nombre);
	cliente->conexion = conexion;
	cliente->mutex_conexion = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(cliente->mutex_conexion, NULL);

	return cliente;
}

void rest_cliente_add(rest_cliente_t* cliente)
{
	pthread_mutex_lock(&mutex_clientes);
	list_add(lista_clientes, (void*) cliente);
	CS_LOG_TRACE("Se agregó el Cliente: {%s}", cliente->nombre);
	pthread_mutex_unlock(&mutex_clientes);
}

void rest_cliente_find(char* cliente, void(*closure)(rest_cliente_t*))
{
	bool _find_cliente_by_name(rest_cliente_t* element) {
		return !strcmp(element->nombre, cliente);
	}

	pthread_mutex_lock(&mutex_clientes);
	closure(list_find(lista_clientes, (void*) _find_cliente_by_name));
	pthread_mutex_unlock(&mutex_clientes);
}

int rest_cliente_find_index(char* cliente)
{
	int index;
	bool _find_cliente_by_name(rest_cliente_t* element) {
		return !strcmp(element->nombre, cliente);
	}

	pthread_mutex_lock(&mutex_clientes);
	index = list_find_index(lista_clientes, (void*) _find_cliente_by_name);
	pthread_mutex_unlock(&mutex_clientes);

	return index;
}
