#include "restclientes.h"

static t_dictionary*   tabla_clientes;
static pthread_mutex_t mutex_clientes;

void rest_clientes_init(void)
{
	tabla_clientes = dictionary_create();
	pthread_mutex_init(&mutex_clientes, NULL);
}

void rest_cliente_connect(char* nombre, t_sfd conexion)
{
	rest_cliente_t* cliente = malloc(sizeof(rest_cliente_t));
	cliente->conexion = conexion;
	pthread_mutex_init(&cliente->mutex_conexion, NULL);

	pthread_mutex_lock(&mutex_clientes);
	dictionary_put(tabla_clientes, nombre, cliente);
	pthread_mutex_unlock(&mutex_clientes);

	CS_LOG_TRACE("Se agregó el Cliente: {ID: %s}", nombre);
}

rest_cliente_t* rest_cliente_get(char* cliente)
{
	rest_cliente_t* info_cliente;

	pthread_mutex_lock(&mutex_clientes);
	info_cliente = dictionary_get(tabla_clientes, cliente);
	pthread_mutex_unlock(&mutex_clientes);

	return info_cliente;
}

bool rest_cliente_is_connected(char* cliente)
{
	pthread_mutex_lock(&mutex_clientes);
	bool esta_conectado = dictionary_has_key(tabla_clientes, cliente);
	pthread_mutex_unlock(&mutex_clientes);

	return esta_conectado ? true : false;
}
