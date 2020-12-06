#ifndef CONN_SEND_H_
#define CONN_SEND_H_

#include "utils/cssockets.h"
#include "utils/csbuffer.h"
#include "csmsgtypes.h"

/**
* @NAME cs_send_msg
* @DESC Envían un mensaje serializado al socket.
* Retornan STATUS_SUCCESS en caso de que el envío haya sido exitoso.
*/
e_status cs_send_handshake_cli(t_sfd conn);
e_status cs_send_handshake_res(t_sfd conn, t_pos pos);
e_status cs_send_consulta (t_sfd conn, e_msgtype msg_type, t_consulta* msg, e_module dest);
e_status cs_send_respuesta(t_sfd conn, t_header header, void* msg);

/**
* @NAME buffer_pack_msg
* @DESC Convierte una estructura mensaje a un único stream.
* ADVERTENCIA: NO USAR, ESTÁ PUBLICADA SOLO PARA TESTS
*/
void buffer_pack_msg(t_buffer* buffer, t_header header, void* msg, e_module dest);

#endif /* CONNSEND_H_ */
