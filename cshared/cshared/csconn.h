#ifndef CONN_CORE_H_
#define CONN_CORE_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "utils/cserror.h"
#include "utils/csconfig.h"

typedef volatile int t_sfd;

typedef struct
{
	int8_t 	 opcode;
	int8_t 	 msgtype;
}t_header;

typedef enum
{
	OPCODE_UNKNOWN = 0,
	OPCODE_CONSULTA,
	OPCODE_RESPUESTA_OK,
	OPCODE_RESPUESTA_FAIL
}e_opcode;

typedef struct
{
	uint32_t size;
	void*	 stream;
}t_buffer;

typedef struct
{
	t_header  header;
	t_buffer* payload;
}t_package;

/**
* @NAME cs_enum_opcode_to_str
* @DESC Devuelve el string correspondiente al enum value de t_enum_msgtype
*/
const char* cs_enum_opcode_to_str(int value);

/**
* @NAME cs_package_destroy
* @DESC Destruye un paquete y su contenido
*/
void cs_package_destroy(t_package* package);

/**
* @NAME cs_buffer_destroy
* @DESC Destruye un buffer y su contenido
*/
void cs_buffer_destroy(t_buffer* buffer);

/**
* @NAME cs_get_peer_info
* @DESC Retorna una IP en formato string desde una dirección a partir de un
 * socket file descriptor
*/
e_status cs_get_peer_info(t_sfd sfd, char** ip_str_ptr, char** port_str_ptr);

/**
* @NAME cs_socket_is_connected
* @DESC Retorna true si el socket está conectado.
*/
bool cs_socket_is_connected(t_sfd sfd);

#endif /* CONNCORE_H_ */
