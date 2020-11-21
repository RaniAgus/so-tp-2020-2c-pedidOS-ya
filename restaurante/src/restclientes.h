#ifndef RESTCLIENTES_H_
#define RESTCLIENTES_H_

#include "restcore.h"

typedef struct
{
	t_sfd conexion;
	pthread_mutex_t mutex_conexion;
}rest_cliente_t;

void rest_clientes_init(void);
void rest_cliente_connect(char* nombre, t_sfd conexion);
rest_cliente_t* rest_cliente_get(char* cliente);
bool rest_cliente_is_connected(char* cliente);

#endif /* RESTCLIENTES_H_ */
