#include "apcore.h"

static t_list* lista_clientes;
static pthread_mutex_t mutex_clientes;

static t_list* lista_restaurantes;
static pthread_mutex_t mutex_restaurantes;

static ap_restaurante_t* restaurante_default;

void ap_core_init(void)
{
	lista_clientes = list_create();
	lista_restaurantes = list_create();

	pthread_mutex_init(&mutex_clientes, NULL);
	pthread_mutex_init(&mutex_restaurantes, NULL);

	t_pos posicion_default = {
			(uint32_t)cs_config_get_int("POSICION_REST_DEFAULT_X"),
			(uint32_t)cs_config_get_int("POSICION_REST_DEFAULT_Y")
	};
	restaurante_default = ap_restaurante_create("Default", posicion_default, NULL, NULL);
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
	restaurante->ip_escucha = ({ ip ? strdup(ip) : NULL; });
	restaurante->puerto_escucha = ({ puerto ? strdup(puerto) : NULL; });

	return restaurante;
}

void ap_cliente_add(ap_cliente_t* cliente)
{
	pthread_mutex_lock(&mutex_clientes);
	list_add(lista_clientes, (void*) cliente);
	CS_LOG_TRACE("Se agregó el Cliente: { %s, (%d:%d) } ", cliente->nombre, cliente->pos.x, cliente->pos.y);
	pthread_mutex_unlock(&mutex_clientes);

}

void ap_restaurante_add(ap_restaurante_t* restaurante)
{
	pthread_mutex_lock(&mutex_restaurantes);
	int index = list_add(lista_restaurantes, (void*) restaurante);
	CS_LOG_TRACE("Se agregó el Restaurante nro.%d: { %s, (%d:%d), %s:%s } ", index,
			restaurante->nombre, restaurante->posicion.x, restaurante->posicion.y,
			restaurante->ip_escucha, restaurante->puerto_escucha
	);
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

void ap_restaurante_get_from_client(char* cliente, void(*closure)(ap_restaurante_t*))
{
	ap_restaurante_t* seleccionado;

	//Para no entrar en un doble mutex, primero obtiene el index
	int index_restaurante;
	void _get_restaurante_index(ap_cliente_t* element) {
		index_restaurante = element->restaurante_seleccionado;
	}
	ap_cliente_find(cliente, _get_restaurante_index);

	//Y después evalúa la lista de restaurantes
	pthread_mutex_lock(&mutex_restaurantes);
	if(!list_is_empty(lista_restaurantes)) {
		//Si hay restaurantes conectados, lo busca con el index obtenido
		seleccionado = list_get(lista_restaurantes, index_restaurante);
	} else
	{
		//Si no hay restaurantes conectados, elige el default
		seleccionado = restaurante_default;
	}
	closure(seleccionado);
	pthread_mutex_unlock(&mutex_restaurantes);
}

void ap_restaurantes_iterate(void(*closure)(ap_restaurante_t*))
{
	pthread_mutex_lock(&mutex_restaurantes);
	list_iterate(lista_restaurantes, (void*) closure);
	pthread_mutex_unlock(&mutex_restaurantes);
}
