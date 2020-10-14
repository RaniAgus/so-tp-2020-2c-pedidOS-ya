#include "apcore.h"

static t_list* lista_clientes;
static pthread_mutex_t mutex_clientes;

static t_list* lista_restaurantes;
static pthread_mutex_t mutex_restaurantes;

void ap_core_init(void)
{
	lista_clientes = list_create();
	lista_restaurantes = list_create();

	pthread_mutex_init(&mutex_clientes, NULL);
	pthread_mutex_init(&mutex_restaurantes, NULL);
}

ap_cliente_t* ap_cliente_create(char* nombre, t_pos posicion, t_sfd conexion)
{
	ap_cliente_t* cliente = malloc(sizeof(ap_cliente_t));

	cliente->nombre = strdup(nombre);
	cliente->pos.x = posicion.x;
	cliente->pos.y = posicion.y;
	cliente->conexion = conexion;
	cliente->restaurante_seleccionado = -1;

	return cliente;
}

ap_restaurante_t* ap_restaurante_create(char* nombre, t_pos posicion, char* ip, char* puerto)
{
	ap_restaurante_t* restaurante = malloc(sizeof(ap_restaurante_t));

	restaurante->nombre = strdup(nombre);
	restaurante->posicion.x = posicion.x;
	restaurante->posicion.y = posicion.y;
	restaurante->ip_escucha = strdup(ip);
	restaurante->puerto_escucha = strdup(puerto);

	return restaurante;
}

void ap_cliente_add(ap_cliente_t* cliente)
{
	pthread_mutex_lock(&mutex_clientes);
	list_add(lista_clientes, (void*) cliente);
	pthread_mutex_unlock(&mutex_clientes);

	CS_LOG_TRACE("Se agregó el Cliente: { %s; (%d,%d) } ", cliente->nombre, cliente->pos.x, cliente->pos.y);
}

void ap_restaurante_add(ap_restaurante_t* restaurante)
{
	pthread_mutex_lock(&mutex_restaurantes);
	list_add(lista_restaurantes, (void*) restaurante);
	pthread_mutex_unlock(&mutex_restaurantes);
}

void ap_cliente_find(char* cliente, void(*closure)(ap_cliente_t*))
{
	bool _find_cliente_by_name(ap_cliente_t* element) {
		return !strcmp(element->nombre, cliente);
	}

	pthread_mutex_lock(&mutex_clientes);
	closure(list_find(lista_clientes, (void*) _find_cliente_by_name));
	pthread_mutex_unlock(&mutex_clientes);
}

int ap_cliente_find_index(char* cliente)
{
	int index;
	bool _find_cliente_by_name(ap_restaurante_t* element) {
		return !strcmp(element->nombre, cliente);
	}

	pthread_mutex_lock(&mutex_clientes);
	index = list_find_index(lista_clientes, (void*) _find_cliente_by_name);
	pthread_mutex_unlock(&mutex_clientes);

	return index;
}

int ap_restaurante_find_index(char* restaurante)
{
	int index;
	bool _find_restaurante_by_name(ap_restaurante_t* element) {
		return !strcmp(element->nombre, restaurante);
	}

	pthread_mutex_lock(&mutex_restaurantes);
	index = list_find_index(lista_restaurantes, (void*) _find_restaurante_by_name);
	pthread_mutex_unlock(&mutex_restaurantes);

	return index;
}
