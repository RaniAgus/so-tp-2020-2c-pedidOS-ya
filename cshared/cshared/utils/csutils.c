#include "csutils.h"

//************************* LISTS *************************

double list_sum(t_list* self, double(*element_value)(void*)) {
	t_link_element* element = self->head;
	double sumatory = 0;

	while(element != NULL) {
		sumatory += element_value(element->data);
		element = element->next;
	}

	return sumatory;
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
	char* str_time = malloc(strlen(format) + 1);
	char* strftime_format = strdup(format);
	struct tm *log_tm = malloc(sizeof(struct tm));

	//time
	time_t log_time = time(NULL);
	if(log_time == -1) {
		return NULL;
	}

	//miliseconds
	struct timeb tmili;
	if(ftime(&tmili)) {
		return NULL;
	}

	//parse
	char* aux = strftime_format;
	while((aux = strstr(aux, "%MS"))) {
		sprintf(aux, "%03hu", tmili.millitm);
		strftime_format[strlen(strftime_format)] = format[strlen(strftime_format)];
		aux = aux + 3;
	}

	localtime_r(&log_time, log_tm);
	strftime(str_time, strlen(format)+1, strftime_format, log_tm);

	free(log_tm);
	free(strftime_format);

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


