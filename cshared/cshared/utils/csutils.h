#ifndef CSHARED_UTILS_CSUTILS_H_
#define CSHARED_UTILS_CSUTILS_H_

#define NON_NULL(...)	__attribute__((nonnull(__VA_ARGS__)))

#define ARGS(...) __VA_ARGS__

#define LAMBDA(return_type, args, code)\
	({ return_type __f__ args code __f__; })

//Standard C
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

//Unix Libs
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <sys/timeb.h>

//Commons
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>

/**
* @NAME list_sum
* @DESC Retorna la sumatoria de todos los elementos de una lista, a través de una
* función 'element_value' que retorna el valor a acumular de cada uno.
*/
double 	list_sum(t_list* self, double(*element_value)(void*));

/**
* @NAME queue_sync_push
* @DESC Inserta un elemento al final de una queue multihilo. Si 'semaphore' es NULL,
* la función es no bloqueante
*/
void	queue_sync_push(t_queue* self, pthread_mutex_t* mutex, sem_t* semaphore, void* data);

/**
* @NAME queue_sync_pop
* @DESC Quita el primer elemento de una queue multihilo. Si 'semaphore' es NULL,
* la función es no bloqueante
*/
void*	queue_sync_pop(t_queue* self, pthread_mutex_t* mutex, sem_t* semaphore);

/**
* @NAME queue_sync_has_elements
* @DESC Devuelve true si la queue multihilo tiene elementos adentro.
*/
bool	queue_sync_has_elements(t_queue* self, pthread_mutex_t* mutex);

/**
* @NAME: cs_temporal_get_string_time
* @DESC: Retorna un string con la hora actual,
* con el formato recibido por parámetro.
* Ejemplos:
* temporal_get_string_time("%d/%m/%y") => "30/09/20"
* temporal_get_string_time("%H:%M:%S:%MS") => "12:51:59:331"
* temporal_get_string_time("%d/%m/%y %H:%M:%S") => "30/09/20 12:51:59"
*/
char* cs_temporal_get_string_time(const char* format);

/**
* @NAME cs_signal_change_action
* @DESC Cambia la acción de la señal 'signal' por un llamado a la función 'new_action',
 * retornando la acción anterior por parámetro.
*/
int cs_signal_change_action(int signal, void (*new_action)(int), struct sigaction* old_action_ptr);


#endif /* CSHARED_UTILS_CSUTILS_H_ */
