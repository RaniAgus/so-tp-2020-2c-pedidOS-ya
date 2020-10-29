#ifndef RESTCLIENTES_H_
#define RESTCLIENTES_H_

#include "restcore.h"

typedef struct
{
	char* nombre;
	t_sfd conexion;
	pthread_mutex_t* mutex_conexion;
}rest_cliente_t;

void rest_clientes_init(void);
rest_cliente_t* rest_cliente_create(char* nombre, t_sfd conexion);

void rest_cliente_add(rest_cliente_t* cliente);
void rest_cliente_get(char* cliente, void(*closure)(rest_cliente_t*));
bool rest_cliente_find(char* cliente);

#endif /* RESTCLIENTES_H_ */
