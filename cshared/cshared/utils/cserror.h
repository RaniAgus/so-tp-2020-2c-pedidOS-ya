#ifndef UTILS_CSERROR_H
#define UTILS_CSERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>

#include "csutils.h"

/**
* @NAME PRINT_ERROR
* @DESC Loguea un mensaje en stderr con el siguiente formato
*
* ERROR_CODE (file:func:line) -- STRING_ERROR
*/
#define PRINT_ERROR(err)\
({  pthread_mutex_lock(cs_logger_get_mutex());\
	console_save_line();\
	fprintf( stderr, "%s#%d (" __FILE__ ":%s:%d) -- %s\n",\
			cs_enum_status_to_str(err), err, __func__ ,__LINE__, cs_string_error(err) );\
	console_restore_line();\
	pthread_mutex_unlock(cs_logger_get_mutex()); })

#define CHECK_STATUS(func) \
({	e_status __val = (func);\
    if(__val != STATUS_SUCCESS) {\
        PRINT_ERROR(__val);\
        exit(__val);} })

typedef enum
{
	STATUS_ADDRINFO_ERROR_OVERFLOW = -12,
	STATUS_ADDRINFO_ERROR_SYSTEM = -11,
	STATUS_ADDRINFO_ERROR_MEMORY = -10,
	STATUS_ADDRINFO_ERROR_SERVICE = -8,
	STATUS_ADDRINFO_ERROR_SOCKTYPE = -7,
	STATUS_ADDRINFO_ERROR_FAMILY = -6,
	STATUS_ADDRINFO_ERROR_FAIL = -4,
	STATUS_ADDRINFO_ERROR_AGAIN = -3,
	STATUS_ADDRINFO_ERROR_NONAME = -2,
	STATUS_ADDRINFO_ERROR_BADFLAGS = -1,

	STATUS_SUCCESS = 0,
	STATUS_LOOK_UP_ERROR = 1,

	STATUS_SIGACTION_ERROR = 2,

	STATUS_CONFIG_ERROR = 3,
	STATUS_LOGGER_ERROR = 4,

	STATUS_CONN_LOST = 5,
	STATUS_SEND_ERROR = 6,
	STATUS_RECV_ERROR = 7,

	STATUS_GETADDRINFO_ERROR = 8,
	STATUS_SOCKET_ERROR = 9,
	STATUS_BIND_ERROR = 10,
	STATUS_LISTEN_ERROR = 11,
	STATUS_CONNECT_ERROR = 12,
	STATUS_ACCEPT_ERROR = 13,

	STATUS_GETPEERNAME_ERROR = 14,
	STATUS_GETNAMEINFO_ERROR = 15
}e_status;

/**
* @NAME cs_enum_status_to_str
* @DESC Devuelve el string correspondiente al enum value de t_enum_status.
*/
const char* cs_enum_status_to_str(int value);

/**
* @NAME cs_string_error
* @DESC Devuelve un string que describe el error, según el t_enum_status y la
 * variable 'cs_err'.
*/
char* cs_string_error(e_status val);

#endif //SHARED_CSERROR_H
