#ifndef APPCONECTADOS_H_
#define APPCONECTADOS_H_

#include <cshared/cshared.h>
#include <commons/collections/dictionary.h>

typedef struct
{
	t_pos posicion;
	char* ip_escucha;
	char* puerto_escucha;
}app_restaurante_t;

typedef struct
{
	t_pos posicion;
	t_sfd conexion;
	char* rest_vinculado;
}app_cliente_t;

void  app_conectados_init(void);

void  app_conectar_cliente(char* nombre, t_pos posicion, t_sfd conexion);
void  app_obtener_cliente(char* cliente, void(*closure)(app_cliente_t*));
bool  app_cliente_esta_conectado(char* cliente);
void  app_iterar_clientes(void(*closure)(char*, app_cliente_t*));
t_pos app_posicion_cliente(char* cliente);

void  app_conectar_restaurante(char* nombre, t_pos posicion, char* ip, char* puerto);
void  app_desconectar_restaurante(char* restaurante);
bool  app_restaurante_esta_conectado(char* restaurante);
char* app_obtener_restaurante_vinculado_a_cliente(char* cliente);
void  app_iterar_restaurantes(void(*closure)(char*, app_restaurante_t*));
bool  app_hay_restaurantes_conectados(void);
t_pos app_posicion_restaurante(char* restaurante);
bool  app_address_restaurante(char* restaurante, char** ip, char** puerto);

#endif /* APPCONECTADOS_H_ */
