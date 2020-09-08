#ifndef CONN_RECV_H_
#define CONN_RECV_H_

#include "utils/cserror.h"
#include "csmsg.h"
#include "csconn.h"

/**
* @NAME cs_recv_msg
* @DESC Recibe un buffer desde la conexión indicada por parámetro, lo deserializa y
* aplica el closure al mensaje recibido. Retorna STATUS_SUCCESS en caso de que se haya
* recibido con éxito.
*/
e_status cs_recv_msg(t_sfd conn, void (*closure)(t_header, void*));

/**
* @NAME cs_buffer_to_msg
* @DESC Recibe un puntero a una estructura buffer y retorna la estructura de
* mensaje correspondiente.
*/
void* cs_buffer_to_msg(t_header header, t_buffer* payload);

#endif /* CONNRECV_H_ */
