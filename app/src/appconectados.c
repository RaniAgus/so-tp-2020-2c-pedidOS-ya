#include "appconectados.h"

static t_dictionary* tabla_clientes;
static pthread_mutex_t mutex_clientes;

static t_dictionary* tabla_restaurantes;
static pthread_mutex_t mutex_restaurantes;

t_pos POSICION_DEFAULT;

//Inicializa los clientes, restaurantes, y lee la posicion default del config.
void app_conectados_init(void)
{
	tabla_clientes = dictionary_create();
	tabla_restaurantes = dictionary_create();

	pthread_mutex_init(&mutex_clientes, NULL);
	pthread_mutex_init(&mutex_restaurantes, NULL);

	POSICION_DEFAULT.x = (uint32_t)cs_config_get_int("POSICION_REST_DEFAULT_X");
	POSICION_DEFAULT.y = (uint32_t)cs_config_get_int("POSICION_REST_DEFAULT_Y");
}

//Agrega un cliente a la lista de clientes.
void app_conectar_cliente(char* nombre, t_pos posicion, t_sfd conexion)
{
	//Crea la estructura cliente con toda la info
	app_cliente_t* cliente = malloc(sizeof(app_cliente_t));
	cliente->posicion.x = posicion.x;
	cliente->posicion.y = posicion.y;
	cliente->conexion = conexion;
	cliente->rest_vinculado = NULL;

	//Lo agrega a la tabla de clientes
	pthread_mutex_lock(&mutex_clientes);
	dictionary_put(tabla_clientes, nombre, cliente);
	CS_LOG_TRACE("Se agregó el Cliente: {ID: %s} {POS_X: %d, POS_Y: %d} ", nombre, cliente->posicion.x, cliente->posicion.y);
	pthread_mutex_unlock(&mutex_clientes);

}

//Agrega un restaurante a la lista de restaurantes.
void app_conectar_restaurante(char* nombre, t_pos posicion, char* ip, char* puerto)
{
	//Crea la estructura restaurante con toda la info
	app_restaurante_t* restaurante = malloc(sizeof(app_restaurante_t));
	restaurante->posicion.x = posicion.x;
	restaurante->posicion.y = posicion.y;
	restaurante->ip_escucha = ({ ip ? strdup(ip) : NULL; });
	restaurante->puerto_escucha = ({ puerto ? strdup(puerto) : NULL; });

	//Lo agrega a la tabla de restaurantes
	pthread_mutex_lock(&mutex_restaurantes);
	dictionary_put(tabla_restaurantes, nombre, restaurante);
	pthread_mutex_unlock(&mutex_restaurantes);
}

//Desvincula un restaurante de un cliente, en caso de estar vinculado, y lo borra usando app_restaurante_destroy.
void app_desconectar_restaurante(char* restaurante)
{
	//Itera la lista de clientes y desvincula el Restaurante si éste está vinculado
	void _desvincular_restaurante(char* key, app_cliente_t* cliente) {
		if(cliente->rest_vinculado != NULL && string_equals_ignore_case(cliente->rest_vinculado, restaurante))
		{
			CS_LOG_TRACE("Se desvincularon: %s <-> %s", key, cliente->rest_vinculado);
			free(cliente->rest_vinculado);
			cliente->rest_vinculado = NULL;
		}
	}
	app_iterar_clientes(_desvincular_restaurante);

	//Quita al restaurante de la tabla de conectados y destruye su info
	pthread_mutex_lock(&mutex_restaurantes);
	void _destruir_info(app_restaurante_t* info_restaurante) {
		free(info_restaurante->ip_escucha);
		free(info_restaurante->puerto_escucha);
		free(info_restaurante);
	}
	dictionary_remove_and_destroy(tabla_restaurantes, restaurante, (void*)_destruir_info);
	pthread_mutex_unlock(&mutex_restaurantes);
}

//Encuentra un cliente de lista_clientes que tenga el nombre pasado como parametro y le aplica la funcion del segundo parametro.
app_cliente_t* app_obtener_cliente(char* cliente)
{
	app_cliente_t* info_cliente;

	pthread_mutex_lock(&mutex_clientes);
	info_cliente = dictionary_get(tabla_clientes, cliente);
	pthread_mutex_unlock(&mutex_clientes);

	return info_cliente;
}

//Devuelve true(1) si el cliente está conectado, false(0) si el cliente no esta conectado
bool app_cliente_esta_conectado(char* cliente)
{
	bool encontro_cliente;

	pthread_mutex_lock(&mutex_clientes);
	encontro_cliente = dictionary_has_key(tabla_clientes, cliente);
	pthread_mutex_unlock(&mutex_clientes);

	return encontro_cliente ? true : false;
}

//Recibe un funcion y se la aplica a todos los clientes de lista_clientes. Es un forEach
void app_iterar_clientes(void(*closure)(char*, app_cliente_t*))
{
	pthread_mutex_lock(&mutex_clientes);
	dictionary_iterator(tabla_clientes, (void*) closure);
	pthread_mutex_unlock(&mutex_clientes);
}

//Devuelve la posición del cliente a partir de su ID
t_pos app_posicion_cliente(char* cliente)
{
	app_cliente_t* encontrado = app_obtener_cliente(cliente);
	return encontrado->posicion;
}

bool app_restaurante_esta_conectado(char* restaurante)
{
	bool encontro_restaurante;

	pthread_mutex_lock(&mutex_restaurantes);
	encontro_restaurante = dictionary_has_key(tabla_restaurantes, restaurante);
	pthread_mutex_unlock(&mutex_restaurantes);

	return encontro_restaurante;
}

//Encuentra el restaurante con el nombre recibido por parametro y devuelve la ip y el puerto para conectarse al mismo
bool app_address_restaurante(char* restaurante, char** ip, char** puerto)
{
	pthread_mutex_lock(&mutex_restaurantes);
	app_restaurante_t* obtenido = dictionary_get(tabla_restaurantes, restaurante);
	if(obtenido)
	{
		*ip = string_duplicate(obtenido->ip_escucha);
		*puerto = string_duplicate(obtenido->puerto_escucha);
	}
	pthread_mutex_unlock(&mutex_restaurantes);

	return obtenido ? true : false;
}

//Similar a app_obtener_copia_restaurante_conectado, pero recibe un cliente y devuelve una copia del restaurante vinculado a ese cliente.
char* app_obtener_restaurante_vinculado_a_cliente(char* cliente)
{
	char* restaurante = NULL;

	if(app_hay_restaurantes_conectados())
	{
		//Obtiene el nombre del restaurante vinculado
		app_cliente_t* info_cliente = app_obtener_cliente(cliente);
		pthread_mutex_lock(&info_cliente->mutex_rest_vinculado);
		if(info_cliente->rest_vinculado) {
			restaurante = string_duplicate(info_cliente->rest_vinculado);
		}
		pthread_mutex_unlock(&info_cliente->mutex_rest_vinculado);

	} else
	{
		//Si no hay, retorna el Default
		restaurante = string_duplicate("Default");
	}

	return restaurante;
}

//Recibe una funcion y se la aplica a todos los restaurantes. Como un forEach.
void app_iterar_restaurantes(void(*closure)(char*, app_restaurante_t*))
{
	pthread_mutex_lock(&mutex_restaurantes);
	dictionary_iterator(tabla_restaurantes, (void*) closure);
	pthread_mutex_unlock(&mutex_restaurantes);
}

//Devuelve true(1) si hay restaurantes conectados, false(0) si no hay ninguno
//Sirve para que el cliente se vincule con "Default" cuando no hay conectados
bool app_hay_restaurantes_conectados(void)
{
	bool hay_conectados;

	pthread_mutex_lock(&mutex_restaurantes);
	hay_conectados = !dictionary_is_empty(tabla_restaurantes);
	pthread_mutex_unlock(&mutex_restaurantes);

	return hay_conectados;
}

//Devuelve la posición del cliente a partir de su nombre
t_pos app_posicion_restaurante(char* restaurante)
{
	t_pos posicion;

	if(!string_equals_ignore_case(restaurante, "Default"))
	{
		pthread_mutex_lock(&mutex_restaurantes);
		app_restaurante_t* encontrado = dictionary_get(tabla_restaurantes, restaurante);
		posicion = encontrado->posicion;
		pthread_mutex_unlock(&mutex_restaurantes);
	} else posicion = POSICION_DEFAULT;

	return posicion;
}
