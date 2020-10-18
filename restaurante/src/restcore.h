#ifndef RESTCORE_H_
#define RESTCORE_H_

#include <cshared/cshared.h>

char* mi_nombre;
t_pos mi_posicion;

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

#endif /* RESTCORE_H_ */
