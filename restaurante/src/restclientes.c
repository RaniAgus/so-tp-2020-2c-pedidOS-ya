#include "restclientes.h"

static t_list* 		   lista_clientes;
static pthread_mutex_t mutex_clientes;

void rest_clientes_init(void)
{
	lista_clientes = list_create();
	pthread_mutex_init(&mutex_clientes, NULL);
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

void rest_cliente_get(char* cliente, void(*closure)(rest_cliente_t*))
{
	bool _find_cliente_by_name(rest_cliente_t* element) {
		return !strcmp(element->nombre, cliente);
	}

	pthread_mutex_lock(&mutex_clientes);
	closure(list_find(lista_clientes, (void*) _find_cliente_by_name));
	pthread_mutex_unlock(&mutex_clientes);
}

bool rest_cliente_find(char* cliente)
{
	bool _find_cliente_by_name(rest_cliente_t* element) {
		return !strcmp(element->nombre, cliente);
	}

	pthread_mutex_lock(&mutex_clientes);
	rest_cliente_t* encontrado = list_find(lista_clientes, (void*) _find_cliente_by_name);
	pthread_mutex_unlock(&mutex_clientes);

	return encontrado ? true : false;
}
