#ifndef UTILS_CSERROR_H
#define UTILS_CSERROR_H

#include "cscore.h"

#define PRINT_ERROR(err)\
({  fprintf( stderr, "%s#%d (" __FILE__ ":%s:%d) -- %s\n",\
	cs_enum_status_to_str(err), err, __func__ ,__LINE__, cs_string_error(err) ); })

#define CHECK_STATUS(func) \
({	e_status __val = (func);\
    if(__val != STATUS_SUCCESS) {\
        PRINT_ERROR(__val);\
        exit(__val);} })

#define SEM_INIT(semaphor, value)\
({ sem_init(&semaphor, 0, value) ?\
({ cs_set_local_err(errno); STATUS_SEM_INIT_ERROR;}) : STATUS_SUCCESS; })

#define PTHREAD_MUTEX_INIT(mutex)\
({ int __val = pthread_mutex_init(&mutex, NULL); (__val != 0 ?\
({ cs_set_local_err(__val); STATUS_PTHREAD_ERROR;}) : STATUS_SUCCESS); })

#define PTHREAD_CREATE(thread, func, arg) \
({ int __val = pthread_create(thread, (void*)NULL, (void*)func, (void*)arg); (__val != 0 ?\
({ cs_set_local_err(__val); STATUS_PTHREAD_ERROR;}) : STATUS_SUCCESS); })

typedef enum
{
	STATUS_SUCCESS = 0,
	STATUS_LOOK_UP_ERROR,

	STATUS_SEM_INIT_ERROR,
	STATUS_PTHREAD_ERROR,
	STATUS_SIGACTION_ERROR,

	STATUS_CONFIG_ERROR,
	STATUS_LOGGER_ERROR,

	STATUS_REJECTED_MSG,
	STATUS_CONN_LOST,
	STATUS_SEND_ERROR,
	STATUS_RECV_ERROR,

	STATUS_GETADDRINFO_ERROR,
	STATUS_SOCKET_ERROR,
	STATUS_BIND_ERROR,
	STATUS_LISTEN_ERROR,
	STATUS_CONNECT_ERROR,
	STATUS_ACCEPT_ERROR,
	STATUS_GETPEERNAME_ERROR

}e_status;

/**
* @NAME cs_enum_status_to_str
* @DESC Devuelve el string correspondiente al enum value de t_enum_status.
*/
const char* cs_enum_status_to_str(int value);

/**
* @NAME cs_error_init
* @DESC Inicia el mutex del estado de error interno.
*/
void cs_error_init(void);

/**
* @NAME cs_error_delete
* @DESC Destruye el mutex del estado de error interno.
*/
void cs_error_delete(void);

/**
* @NAME cs_string_error
* @DESC Devuelve un string que describe el error, según el t_enum_status y la
 * variable 'cs_err'.
*/
char* cs_string_error(e_status val);

/**
* @NAME cs_set_err
* @DESC Asigna un valor a la variable 'cs_err'.
*/
void cs_set_local_err(int val);

#endif //SHARED_CSERROR_H
