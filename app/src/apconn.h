#ifndef APCONN_H_
#define APCONN_H_

#include <cshared/cshared.h>

typedef struct
{
	char* nombre;
	t_pos pos;
	int restaurante_seleccionado;
	t_sfd conexion;
}ap_cliente_t;

typedef struct
{
	char* nombre;
	t_pos posicion;
	char* ip_escucha;
	char* puerto_escucha;
}ap_restaurante_t;

void ap_core_init(void);

ap_cliente_t* ap_cliente_create(char* nombre, t_pos posicion, t_sfd conexion);
void 		  ap_cliente_add(ap_cliente_t* cliente);
void 		  ap_cliente_find(char* cliente, void(*closure)(ap_cliente_t*));
int  		  ap_cliente_find_index(char* cliente);

ap_restaurante_t* ap_restaurante_create(char* nombre, t_pos posicion, char* ip, char* puerto);
void 			  ap_restaurante_add(ap_restaurante_t* restaurante);
void 			  ap_restaurante_get_from_client(char* cliente, void(*closure)(ap_restaurante_t*));
int 			  ap_restaurante_find_index(char* restaurante);
void 			  ap_restaurantes_iterate(void(*closure)(ap_restaurante_t*));

#endif /* APCONN_H_ */
