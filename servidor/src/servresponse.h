#ifndef SRC_SERVRESPONSE_H_
#define SRC_SERVRESPONSE_H_

#include <cshared/cshared.h>

void server_send_rta_consultar_restaurantes(t_sfd client_conn);
void server_send_rta_obtener_restaurante(t_sfd client_conn);
void server_send_rta_consultar_platos(t_sfd client_conn);
void server_send_rta_crear_pedido(t_sfd client_conn);
void server_send_rta_consultar_pedido(t_sfd client_conn);
void server_send_rta_obtener_pedido(t_sfd client_conn);
void server_send_rta_obtener_receta(t_sfd client_conn);
void server_send_rta_ok(e_msgtype msg_type, t_sfd client_conn);

#endif /* SRC_SERVRESPONSE_H_ */
