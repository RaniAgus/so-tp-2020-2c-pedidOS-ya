#ifndef APPCONECTADOS_H_
#define APPCONECTADOS_H_

#include <cshared/cshared.h>

typedef struct
{
	char* nombre;
	t_pos posicion;
	char* ip_escucha;
	char* puerto_escucha;
}app_restaurante_t;

typedef struct
{
	char* nombre;
	t_pos posicion;
	t_sfd conexion;
	pthread_mutex_t mutex_conexion;
	app_restaurante_t* rest_vinculado;
}app_cliente_t;

void app_conectados_init(void);

app_cliente_t* 	   app_cliente_create(char* nombre, t_pos posicion, t_sfd conexion);
app_restaurante_t* app_restaurante_create(char* nombre, t_pos posicion, char* ip, char* puerto);

void app_restaurante_destroy(app_restaurante_t* restaurante);

void app_agregar_cliente(app_cliente_t* cliente);
void app_obtener_cliente(char* cliente, void(*closure)(app_cliente_t*));
bool app_cliente_esta_conectado(char* cliente);
void app_iterar_clientes(void(*closure)(app_cliente_t*));

int   			   app_agregar_restaurante(app_restaurante_t* restaurante);
void			   app_quitar_y_desvincular_restaurante(char* restaurante);
app_restaurante_t* app_obtener_copia_restaurante_conectado(char* nombre);
app_restaurante_t* app_obtener_copia_restaurante_vinculado_a_cliente(char* cliente);
void 			   app_iterar_restaurantes(void(*closure)(app_restaurante_t*));
bool			   app_hay_restaurantes_conectados(void);

#endif /* APPCONECTADOS_H_ */
