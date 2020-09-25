#include "cshared.h"

void cs_parse_argument(char* arg)
{
	char *arg_without_dashes, **key_and_value;

	arg_without_dashes = string_substring_from(arg, 2);
	key_and_value = string_n_split(arg_without_dashes, 2, "=");

	if(!strcmp(key_and_value[0],"log-level"))
	{
		cs_logger_set_level(log_level_from_string(key_and_value[1]));
	}

	string_iterate_lines(key_and_value,(void*) free);
	free(key_and_value);
	free(arg_without_dashes);
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
