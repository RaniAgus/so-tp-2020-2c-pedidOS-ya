#include "appconectados.h"

static t_list* lista_clientes;
static pthread_mutex_t mutex_clientes;

static t_list* lista_restaurantes;
static pthread_mutex_t mutex_restaurantes;

t_pos posicion_default;

void app_conectados_init(void)
{
	lista_clientes = list_create();
	lista_restaurantes = list_create();

	pthread_mutex_init(&mutex_clientes, NULL);
	pthread_mutex_init(&mutex_restaurantes, NULL);

	posicion_default.x = (uint32_t)cs_config_get_int("POSICION_REST_DEFAULT_X");
	posicion_default.y = (uint32_t)cs_config_get_int("POSICION_REST_DEFAULT_Y");
}

app_cliente_t* app_cliente_create(char* nombre, t_pos posicion, t_sfd conexion)
{
	app_cliente_t* cliente = malloc(sizeof(app_cliente_t));

	cliente->nombre = strdup(nombre);
	cliente->posicion.x = posicion.x;
	cliente->posicion.y = posicion.y;
	cliente->conexion = conexion;
	cliente->rest_vinculado = NULL;
	pthread_mutex_init(&cliente->mutex_conexion, NULL);

	return cliente;
}

app_restaurante_t* app_restaurante_create(char* nombre, t_pos posicion, char* ip, char* puerto)
{
	app_restaurante_t* restaurante = malloc(sizeof(app_restaurante_t));

	restaurante->nombre = strdup(nombre);
	restaurante->posicion.x = posicion.x;
	restaurante->posicion.y = posicion.y;
	restaurante->ip_escucha = ({ ip ? strdup(ip) : NULL; });
	restaurante->puerto_escucha = ({ puerto ? strdup(puerto) : NULL; });

	return restaurante;
}

void app_restaurante_destroy(app_restaurante_t* restaurante)
{
	if(restaurante != NULL) {
		free(restaurante->nombre);
		if(restaurante->ip_escucha != NULL)
			free(restaurante->ip_escucha);
		if(restaurante->puerto_escucha != NULL)
			free(restaurante->puerto_escucha);
		free(restaurante);
	}
}

void app_agregar_cliente(app_cliente_t* cliente)
{
	pthread_mutex_lock(&mutex_clientes);
	list_add(lista_clientes, (void*) cliente);
	CS_LOG_TRACE("Se agregó el Cliente: { %s, (%d:%d) } ", cliente->nombre, cliente->posicion.x, cliente->posicion.y);
	pthread_mutex_unlock(&mutex_clientes);

}

int app_agregar_restaurante(app_restaurante_t* restaurante)
{
	pthread_mutex_lock(&mutex_restaurantes);
	int index = list_add(lista_restaurantes, (void*) restaurante);
	pthread_mutex_unlock(&mutex_restaurantes);

	return index;
}

void app_quitar_y_desvincular_restaurante(char* restaurante)
{
	pthread_mutex_lock(&mutex_restaurantes);
	bool _restaurante_compare(app_restaurante_t* elemento) {
		return !strcmp(elemento->nombre, restaurante);
	}
	list_remove_and_destroy_by_condition(lista_restaurantes, (void*) _restaurante_compare, (void*) app_restaurante_destroy);
	pthread_mutex_unlock(&mutex_restaurantes);

	//Itera la lista de clientes y desvincula el Restaurante si éste está vinculado
	void _desvincular_restaurante(app_cliente_t* cliente) {
		if(cliente->rest_vinculado != NULL && !strcmp(cliente->rest_vinculado->nombre, restaurante)) {
			CS_LOG_TRACE("Se desvincularon: %s <-> %s", cliente->nombre, cliente->rest_vinculado->nombre);
			app_restaurante_destroy(cliente->rest_vinculado);
			cliente->rest_vinculado = NULL;
		}
	}
	app_iterar_clientes(_desvincular_restaurante);
}

void app_obtener_cliente(char* cliente, void(*closure)(app_cliente_t*))
{
	bool _find_cliente_by_name(app_cliente_t* element) {
		return !strcmp(element->nombre, cliente);
	}

	pthread_mutex_lock(&mutex_clientes);
	closure(list_find(lista_clientes, (void*) _find_cliente_by_name));
	pthread_mutex_unlock(&mutex_clientes);
}

bool app_cliente_esta_conectado(char* cliente)
{
	app_cliente_t* encontrado;

	bool _find_cliente_by_name(app_cliente_t* element) {
		return !strcmp(element->nombre, cliente);
	}
	pthread_mutex_lock(&mutex_clientes);
	encontrado = list_find(lista_clientes, (void*) _find_cliente_by_name);
	pthread_mutex_unlock(&mutex_clientes);

	return encontrado ? true : false;
}

void app_iterar_clientes(void(*closure)(app_cliente_t*))
{
	pthread_mutex_lock(&mutex_clientes);
	list_iterate(lista_clientes, (void*) closure);
	pthread_mutex_unlock(&mutex_clientes);
}

app_restaurante_t* app_obtener_copia_restaurante_conectado(char* restaurante)
{
	app_restaurante_t* restaurante_obtenido = NULL;
	app_restaurante_t* encontrado;

	bool _find_restaurante_by_name(app_restaurante_t* element) {
		return !strcmp(element->nombre, restaurante);
	}

	pthread_mutex_lock(&mutex_restaurantes);
	encontrado = list_find(lista_restaurantes, (void*) _find_restaurante_by_name);
	if(encontrado != NULL)
	{
		restaurante_obtenido = app_restaurante_create(
			encontrado->nombre,
			encontrado->posicion,
			encontrado->ip_escucha,
			encontrado->puerto_escucha
		);
	}
	pthread_mutex_unlock(&mutex_restaurantes);

	return restaurante_obtenido;
}

app_restaurante_t* app_obtener_copia_restaurante_vinculado_a_cliente(char* cliente)
{
	app_restaurante_t* seleccionado = NULL;

	if(app_hay_restaurantes_conectados())
	{
		//Si hay restaurantes conectados, lo busca desde el cliente
		void _get_restaurante(app_cliente_t* encontrado) {
			if(encontrado->rest_vinculado != NULL) {
				seleccionado = app_restaurante_create(
						encontrado->rest_vinculado->nombre,
						encontrado->rest_vinculado->posicion,
						encontrado->rest_vinculado->ip_escucha,
						encontrado->rest_vinculado->puerto_escucha
				);
			}
		}
		app_obtener_cliente(cliente, _get_restaurante);
	} else
	{
		//Si no hay, retorna el Default
		seleccionado = app_restaurante_create("Default", posicion_default, NULL, NULL);
	}

	return seleccionado;
}

void app_iterar_restaurantes(void(*closure)(app_restaurante_t*))
{
	pthread_mutex_lock(&mutex_restaurantes);
	list_iterate(lista_restaurantes, (void*) closure);
	pthread_mutex_unlock(&mutex_restaurantes);
}

bool app_hay_restaurantes_conectados(void)
{
	bool result;

	pthread_mutex_lock(&mutex_restaurantes);
	result = !list_is_empty(lista_restaurantes);
	pthread_mutex_unlock(&mutex_restaurantes);

	return result;
}
