#include "csutils.h"

//************************* LISTS *************************
static void* list_fold_elements(t_link_element* element, void* seed, void*(*operation)(void*, void*));
double list_sum(t_list* self, double(*element_value)(void*)) {
	t_link_element* element = self->head;
	double sumatory = 0;

	while(element != NULL) {
		sumatory += element_value(element->data);
		element = element->next;
	}

	return sumatory;
}

void* list_fold1(t_list* self, void* (*operation)(void*, void*)) {
	return self->elements_count > 0 ? list_fold_elements(self->head->next, self->head->data, operation) : NULL;
}

void* list_get_min_by(t_list* self, int (*comparator)(void*, void*)) {
	void* _return_minimum(void* seed, void* data) {
		return comparator(seed, data) <= 0 ? seed : data;
	}
	return list_fold1(self, _return_minimum);
}

static void* list_fold_elements(t_link_element* element, void* seed, void*(*operation)(void*, void*)) {
	void* result = seed;
	while(element != NULL) {
		result = operation(result, element->data);
		element = element->next;
	}

	return result;
}

//************************* QUEUES *************************

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

bool	queue_sync_has_elements(t_queue* self, pthread_mutex_t* mutex)
{
	pthread_mutex_lock(mutex);
	bool is_empty = queue_is_empty(self);
	pthread_mutex_unlock(mutex);

	return !is_empty;
}

//************************* TIME *************************

char* cs_temporal_get_string_time(const char* format) {
	char* str_time = strdup(format);

	struct timespec* log_timespec = malloc(sizeof(struct timespec));
	struct tm* log_tm = malloc(sizeof(struct tm));
	char* milisec;

	if(clock_gettime(CLOCK_REALTIME, log_timespec) == -1) {
		return NULL;
	}
	milisec = string_from_format("%03d", log_timespec->tv_nsec / 1000000);

	for(char* ms = strstr(str_time, "%MS"); ms != NULL; ms = strstr(ms + 3, "%MS")) {
		memcpy(ms, milisec, 3);
	}

	localtime_r(&log_timespec->tv_sec, log_tm);
	strftime(str_time, strlen(format) + 1, str_time, log_tm);

	free(milisec);
	free(log_tm);
	free(log_timespec);

	return str_time;
}

//************************* SIGNAL *************************

int cs_signal_change_action(int signal, void (*new_action)(int), struct sigaction* old_action_ptr)
{
	struct sigaction sigint_action;
	memset(&sigint_action, 0, sizeof(sigint_action));

	sigint_action.sa_handler = new_action;

	return sigaction(signal, &sigint_action, old_action_ptr);
}


