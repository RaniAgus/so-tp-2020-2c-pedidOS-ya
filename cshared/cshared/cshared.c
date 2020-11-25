#include "cshared.h"

void cs_parse_argument(char* arg)
{
	char **key_and_value = string_n_split(arg + 1, 2, "=");

	switch(key_and_value[0][0])
	{
	case 'l': cs_logger_set_level(log_level_from_string(key_and_value[1])); break;
	case 'c': cs_logger_set_console(atoi(key_and_value[1]));
	}

	string_iterate_lines(key_and_value,(void*) free);
	free(key_and_value);
}

void cs_module_init(const char* config_file_path, const char* log_file_key, const char* module_name)
{
	CHECK_STATUS(cs_config_init(config_file_path));
	CHECK_STATUS(cs_logger_init(log_file_key, module_name));
	cs_error_init();
}

void cs_module_close(void)
{
	cs_error_delete();
	cs_logger_delete();
	cs_config_delete();
}
