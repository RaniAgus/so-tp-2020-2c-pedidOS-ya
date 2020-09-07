#ifndef CONN_CREATE_H_
#define CONN_CREATE_H_

#include "csconn.h"
#include "utils/csstring.h"

/**
* @NAME cs_tcp_server_accept_routine
* @DESC Acepta conexiones al socket file descriptor que recibe por parámetro, y, con
* esa conexión aceptada, realiza la función que recibe por parámetro. En caso de error,
* llama a la función que lo maneja, pasándole por parámetro el tipo de error.
*
* NOTA: Para utilizar funciones que reciban más parámetros como "success_action()", usa
* orden superior.
*/
void cs_tcp_server_accept_routine(t_sfd* conn, void(*success_action)(t_sfd), void(*err_handler)(e_status));

/**
* @NAME cs_tcp_server_create
* @DESC Crea un servidor (recibiendo por parámetro la dirección ip y el
* puerto) y devuelve el socket file descriptor correspondiente. Si no logra crearlo,
* retorna el tipo de error.
*/
e_status cs_tcp_server_create(t_sfd* conn, char* ip, char* port);

/**
* @NAME cs_tcp_client_create
* @DESC Se conecta a un servidor (recibiendo por parámetro la dirección ip y el
* puerto) y devuelve el socket file descriptor correspondiente. Si no logra conectarse,
* retorna el tipo de error.
*/
e_status cs_tcp_client_create(t_sfd* conn, char* ip, char* port);

#endif /* CONNCREATE_H_ */
