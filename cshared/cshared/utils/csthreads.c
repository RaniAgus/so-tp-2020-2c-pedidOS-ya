#include "csthreads.h"

void	queue_sync_push(t_queue* self, pthread_mutex_t* mutex, sem_t* semaphor, void* data)
{
	pthread_mutex_lock(mutex);
	queue_push(self, data);
	pthread_mutex_unlock(mutex);
	if(semaphor) sem_post(semaphor);
}

void*	queue_sync_pop(t_queue* self, pthread_mutex_t* mutex, sem_t* semaphor)
{
	void* data = NULL;

	if(semaphor) sem_wait(semaphor);
	pthread_mutex_lock(mutex);
	if(!queue_is_empty(self)) data = queue_pop(self);
	pthread_mutex_unlock(mutex);

	return data;
}
