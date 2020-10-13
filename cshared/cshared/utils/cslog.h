#ifndef UTILS_LOG_H_
#define UTILS_LOG_H_

#include "cserror.h"
#include "csconfig.h"
#include "csconsole.h"
#include <commons/log.h>

/**
* @NAME cs_logger_init (versión de log_create de commons/log.h)
* @DESC Crea la instancia de logger interna, tomando por parametro
* el nombre del programa y el nombre del archivo donde se van a generar los logs.
*/
e_status cs_logger_init(const char* file, const char* program_name);

/**
* @NAME cs_logger_delete (versión de log_destroy de commons/log.h)
* @DESC Destruye la instancia de logger interna.
*/
void   		  cs_logger_delete(void);

/**
* @NAME cs_logger_set_level
* @DESC Cambia el nivel de log interno (antes de iniciar el log).
*/
void		 cs_logger_set_level(t_log_level level);

/**
* @NAME cs_logger_get_mutex
* @DESC Retorna el mutex de la instancia de logger interna.
*/
pthread_mutex_t* cs_logger_get_mutex(void);

/**
* @NAME cs_logger_get
* @DESC Retorna la instancia de logger interna.
*/
t_log*           cs_logger_get(void);

/**
* @NAME CS_LOG_TRACE (versión de log_trace de commons/log.h)
* @DESC Loguea un mensaje con el siguiente formato
*
* [TRACE] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
*/
#define CS_LOG_TRACE(...) do {\
	pthread_mutex_lock(cs_logger_get_mutex());\
	console_save_line();\
	log_trace(cs_logger_get(), __VA_ARGS__);\
	console_restore_line();\
	pthread_mutex_unlock(cs_logger_get_mutex()); } while(0)

/**
* @NAME CS_LOG_DEBUG (versión de log_debug de commons/log.h)
* @DESC Loguea un mensaje con el siguiente formato
*
* [DEBUG] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
*/
#define CS_LOG_DEBUG(...) do {\
	pthread_mutex_lock(cs_logger_get_mutex());\
	console_save_line();\
	log_debug(cs_logger_get(), __VA_ARGS__);\
	console_restore_line();\
	pthread_mutex_unlock(cs_logger_get_mutex()); }while(0)

/**
* @NAME CS_LOG_INFO (versión de log_info de commons/log.h)
* @DESC Loguea un mensaje con el siguiente formato
*
* [INFO] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
*/
#define CS_LOG_INFO(...) do {\
	pthread_mutex_lock(cs_logger_get_mutex());\
	console_save_line();\
	log_info(cs_logger_get(), __VA_ARGS__);\
	console_restore_line();\
	pthread_mutex_unlock(cs_logger_get_mutex()); } while(0)

/**
* @NAME CS_LOG_WARNING (versión de log_warning de commons/log.h)
* @DESC Loguea un mensaje con el siguiente formato
*
* [WARNING] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
*/
#define CS_LOG_WARNING(...) do {\
	pthread_mutex_lock(cs_logger_get_mutex());\
	console_save_line();\
	log_warning(cs_logger_get(), __VA_ARGS__);\
	console_restore_line();\
	pthread_mutex_unlock(cs_logger_get_mutex()); } while(0)

/**
* @NAME CS_LOG_ERROR (versión de log_error de commons/log.h)
* @DESC Loguea un mensaje con el siguiente formato
*
* [ERROR] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
*/
#define CS_LOG_ERROR(...) do {\
	pthread_mutex_lock(cs_logger_get_mutex());\
	console_save_line();\
	log_error(cs_logger_get(), __VA_ARGS__);\
	console_restore_line();\
	pthread_mutex_unlock(cs_logger_get_mutex()); } while(0)

#endif /* UTILS_LOG_H_ */
