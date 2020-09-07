#ifndef CONN_SEND_H_
#define CONN_SEND_H_

#include "utils/cserror.h"
#include "utils/csmsg.h"
#include "csconn.h"
#include "csconncreate.h"

/**
* @NAME db_send_and_recv_ack
* @DESC Envía un mensaje serializado a la IP y PUERTO que lee desde el archivo
* config interno, retornando la respuesta ack del receptor. Retorna STATUS_SUCCESS
* en caso de que el envío y recepción haya sido exitoso.
*
*/
e_status cs_connect_and_send_msg(const char* ip_key, const char* port_key,
									t_header header, void* msg, uint32_t* ack_ptr);

/**
* @NAME cs_send
* @DESC Envía a la conexión un mensaje serializado, recibiendo sus componentes
* por argumento. Retorna STATUS_SUCCESS en caso de que se haya enviado con éxito.
*
*/
e_status cs_send(t_sfd conn, t_header header, t_buffer* payload);

/**
* @NAME cs_recv_ack
* @DESC Recibe el ack de un mensaje enviado. Retorna STATUS_SUCCESS en caso
* de que se haya recibido con éxito.
*/
e_status cs_recv_ack(t_sfd conn, uint32_t* id);


/**
* @NAME cs_msg_to_buffer
* @DESC Recibe un puntero a una estructura mensaje, arma un stream serializado con
* él y lo retorna junto con su tamaño en un puntero a una estructura buffer.
*/
t_buffer*  cs_msg_to_buffer(t_header header, void* msg);

#endif /* CONNSEND_H_ */
