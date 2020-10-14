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
e_status cs_send_handshake_cli(t_sfd conn);
e_status cs_send_handshake_res(t_sfd conn, t_pos pos);
e_status cs_send_consulta (t_sfd conn, e_msgtype msg_type, t_consulta* msg, e_module dest);
e_status cs_send_respuesta(t_sfd conn, t_header  header, void* msg);


#endif /* CONNSEND_H_ */
