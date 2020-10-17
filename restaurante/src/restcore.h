#ifndef RESTCORE_H_
#define RESTCORE_H_

#include <cshared/cshared.h>

char* mi_nombre;
t_pos mi_posicion;

typedef struct
{
	char* nombre;
	t_sfd conexion;
	pthread_mutex_t* mutex_conexion;
}rest_cliente_t;

void rest_core_init(t_rta_obt_rest* metadata);

uint32_t 		rest_generar_id(void);
rest_cliente_t* rest_cliente_create(char* nombre, t_sfd conexion);

void rest_cliente_add(rest_cliente_t* cliente);
void rest_cliente_find(char* cliente, void(*closure)(rest_cliente_t*));
int  rest_cliente_find_index(char* cliente);

#endif /* RESTCORE_H_ */
