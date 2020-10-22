#ifndef CSHARED_UTILS_CSTHREADS_H_
#define CSHARED_UTILS_CSTHREADS_H_

#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/queue.h>

void	queue_sync_push(t_queue* self, pthread_mutex_t* mutex, sem_t* semaphore, void* data);
void*	queue_sync_pop(t_queue* self, pthread_mutex_t* mutex, sem_t* semaphore);
bool	queue_sync_has_elements(t_queue* self, pthread_mutex_t* mutex);

#endif /* CSHARED_UTILS_CSTHREADS_H_ */
