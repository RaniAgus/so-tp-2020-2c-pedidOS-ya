#ifndef CONN_RECV_H_
#define CONN_RECV_H_

#include "utils/cssockets.h"
#include "csmsgtypes.h"

/**
* @NAME cs_recv_msg
* @DESC Recibe un buffer desde la conexión indicada por parámetro, lo deserializa y
* aplica el closure al mensaje recibido. Retorna STATUS_SUCCESS en caso de que se haya
* recibido con éxito.
*/
e_status cs_recv_msg(t_sfd conn, void (*closure)(t_sfd, t_header, void*));

/**
* @NAME cs_buffer_to_msg
* @DESC Convierten un stream a una estructura mensaje.
* ADVERTENCIA: NO USAR, ESTÁN PUBLICADAS SOLO PARA TESTS
*
*/
void* cs_buffer_to_msg(t_header header, t_buffer* payload, t_sfd conn);

#endif /* CONNRECV_H_ */
