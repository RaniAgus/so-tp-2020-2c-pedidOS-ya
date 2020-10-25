#ifndef CONN_SEND_H_
#define CONN_SEND_H_

#include "utils/cserror.h"
#include "csmsg.h"
#include "csconn.h"

/**
* @NAME cs_send_msg
* @DESC Envían un mensaje serializado al socket.
* Retornan STATUS_SUCCESS en caso de que el envío haya sido exitoso.
*
*/
e_status cs_send_handshake_cli(t_sfd conn);
e_status cs_send_handshake_res(t_sfd conn, t_pos pos);
e_status cs_send_consulta (t_sfd conn, e_msgtype msg_type, t_consulta* msg, e_module dest);
e_status cs_send_respuesta(t_sfd conn, t_header  header, void* msg);

/**
* @NAME cs_msg_to_buffer
* @DESC Convierten una estructura mensaje a un único stream.
* ADVERTENCIA: NO USAR, ESTÁN PUBLICADAS SOLO PARA TESTS
*
*/
t_buffer*  cs_consulta_to_buffer(t_consulta* msg, e_module dest);
t_buffer*  cs_handshake_cli_to_buffer(t_handshake_cli* msg);
t_buffer*  cs_handshake_res_to_buffer(t_handshake_res* msg);
t_buffer*  cs_respuesta_to_buffer(t_header header, void* msg);

#endif /* CONNSEND_H_ */
