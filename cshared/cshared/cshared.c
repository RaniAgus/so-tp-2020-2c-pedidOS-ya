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
