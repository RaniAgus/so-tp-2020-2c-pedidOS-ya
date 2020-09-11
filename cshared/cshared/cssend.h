#ifndef CONN_SEND_H_
#define CONN_SEND_H_

#include "utils/cserror.h"
#include "csmsg.h"
#include "csconn.h"
#include "csconncreate.h"

/**
* @NAME cs_send_msg
* @DESC Envía un mensaje serializado al socket.
* Retorna STATUS_SUCCESS en caso de que el envío haya sido exitoso.
*
*/
e_status cs_send_msg(t_sfd conn, t_header header, void* msg);

/**
* @NAME cs_msg_to_buffer
* @DESC Recibe un puntero a una estructura mensaje, arma un stream serializado con
* él y lo retorna junto con su tamaño en un puntero a una estructura buffer.
*/
t_buffer*  cs_msg_to_buffer(t_header header, void* msg);

#endif /* CONNSEND_H_ */
