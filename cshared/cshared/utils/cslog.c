#include "cslog.h"

static t_log*           CS_LOGGER_INTERNAL = NULL;
static pthread_mutex_t  CS_LOGGER_MUTEX;
static t_log_level      CS_LOGGER_LEVEL = LOG_LEVEL_INFO;
static int              CS_CONSOLE_MODE = 1;

e_status cs_logger_init(const char* file_key, const char* program_name)
{
	char* file = NULL;

	pthread_mutex_init(&CS_LOGGER_MUTEX, NULL);

	file = cs_config_get_string(file_key);
	if(file == NULL) 
		return STATUS_LOGGER_ERROR;
	

	CS_LOGGER_INTERNAL = log_create(file, (char*)program_name, CS_CONSOLE_MODE, CS_LOGGER_LEVEL);

	return (CS_LOGGER_INTERNAL) ? STATUS_SUCCESS : STATUS_LOGGER_ERROR;
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

bool cs_logger_allows_level(t_log_level log_level)
{
	return log_level >= CS_LOGGER_INTERNAL->detail;
}

void cs_log_hexdump(t_log_level log_level, void* source, size_t length)
{
	char *log_colors[] = {"\x1b[36m", "\x1b[32m", "", "\x1b[33m", "\x1b[31m" };

	if(cs_logger_allows_level(log_level))
	{
		pthread_mutex_lock(cs_logger_get_mutex());
		console_save_line();

		char* buffer = mem_hexstring(source, length);
		string_append(&buffer, "\n");
		if(CS_LOGGER_INTERNAL->file != NULL) {
			txt_write_in_file(CS_LOGGER_INTERNAL->file, buffer);
		}
		if(CS_LOGGER_INTERNAL->is_active_console) {
			printf("%s%s\x1b[0m", log_colors[log_level], buffer);
			fflush(stdout);
		}
		free(buffer);

		console_restore_line();
		pthread_mutex_unlock(cs_logger_get_mutex());
	}
}
