#ifndef CONN_RECV_H_
#define CONN_RECV_H_

#include "utils/cserror.h"
#include "utils/csmsg.h"
#include "csconn.h"

/**
* @NAME cs_recv_msg
* @DESC Recibe un buffer desde la conexión indicada por parámetro, lo deserializa y
* aplica el closure al mensaje recibido. Retorna STATUS_SUCCESS en caso de que se haya
* recibido con éxito.
*/
e_status cs_recv_msg(t_sfd conn, uint32_t (*closure)(t_header, void*));

/**
* @NAME cs_recv_header
* @DESC Recibe el encabezado de un mensaje. Retorna STATUS_SUCCESS en caso de que se
* haya recibido con éxito.
*/
e_status cs_recv_header(t_sfd sfd_cliente, t_header* header) NON_NULL(2);

/**
* @NAME cs_recv_payload
* @DESC Recibe el payload de un mensaje. Retorna STATUS_SUCCESS en caso de que se
* haya recibido con éxito.
*/
e_status cs_recv_payload(t_sfd sfd_cliente, t_buffer* payload) NON_NULL(2);

/**
* @NAME cs_send_ack
* @DESC Envía un ack a la conexión de la que se recibió un mensaje. Retorna
* STATUS_SUCCESS en caso de que se haya enviado con éxito.
*/
e_status cs_send_ack(e_status conn, uint32_t ack);

/**
* @NAME cs_buffer_to_msg
* @DESC Recibe un puntero a una estructura buffer y retorna la estructura de
* mensaje correspondiente.
*/
void* cs_buffer_to_msg(t_header header, t_buffer* payload);

#endif /* CONNRECV_H_ */
