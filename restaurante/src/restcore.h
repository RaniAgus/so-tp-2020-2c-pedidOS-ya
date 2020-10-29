#ifndef RESTCORE_H_
#define RESTCORE_H_

#include <cshared/cshared.h>

char* mi_nombre;
t_pos mi_posicion;

typedef enum { ESTADO_NEW = 1, ESTADO_READY, ESTADO_BLOCK, ESTADO_EXEC, ESTADO_EXIT } rest_estado_e;

typedef struct
{
	t_list*			comidas;
	t_queue* 		queue;
	pthread_mutex_t mutex_queue;
}rest_cola_ready_t;

typedef struct
{
	t_queue* 		queue;
	pthread_mutex_t mutex_queue;
}rest_cola_io_t;

const char* rest_estado_to_str(rest_estado_e value);

#endif /* RESTCORE_H_ */
