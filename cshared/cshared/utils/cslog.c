#include "cslog.h"

static t_log*           CS_LOGGER_INTERNAL = NULL;
static pthread_mutex_t  CS_LOGGER_MUTEX;
static t_log_level 		CS_LOGGER_LEVEL = LOG_LEVEL_TRACE;
static int				CS_CONSOLE_MODE = 1;

e_status cs_logger_init(const char* file_key, const char* program_name)
{
	char* file = NULL;

	pthread_mutex_init(&CS_LOGGER_MUTEX, NULL);

	file = cs_config_get_string(file_key);
	if(file == NULL) 
	{
		cs_set_local_err(errno);
		return STATUS_LOGGER_ERROR;
	}

	CS_LOGGER_INTERNAL = log_create(file, (char*)program_name, CS_CONSOLE_MODE, CS_LOGGER_LEVEL);

	return (CS_LOGGER_INTERNAL) ? STATUS_SUCCESS : ({cs_set_local_err(errno); STATUS_LOGGER_ERROR;});
}

void cs_logger_delete(void)
{
	log_destroy(CS_LOGGER_INTERNAL);
	pthread_mutex_destroy(&CS_LOGGER_MUTEX);
}

void cs_logger_set_level(t_log_level level)
{
	CS_LOGGER_LEVEL = level;
}

void cs_logger_set_console(int value)
{
	CS_CONSOLE_MODE = value;
}

t_log* cs_logger_get(void)
{
	return CS_LOGGER_INTERNAL;
}

pthread_mutex_t* cs_logger_get_mutex(void)
{
	return &CS_LOGGER_MUTEX;
}
