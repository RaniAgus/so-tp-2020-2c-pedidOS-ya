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
e_status cs_send_handshake(t_sfd conn, e_msgtype msg_type, t_handshake* msg);
e_status cs_send_consulta (t_sfd conn, e_msgtype msg_type, t_consulta* msg, e_module dest);
e_status cs_send_respuesta(t_sfd conn, t_header  header, void* msg);

/**
* @NAME cs_msg_to_buffer
* @DESC Recibe un puntero a una estructura mensaje, arma un stream serializado con
* él y lo retorna junto con su tamaño en un puntero a una estructura buffer.
*/
t_buffer*  cs_consulta_to_buffer(t_consulta* msg, e_module dest);
t_buffer*  cs_handshake_to_buffer(t_handshake* msg);
t_buffer*  cs_respuesta_to_buffer(t_header header, void* msg);


#endif /* CONNSEND_H_ */
