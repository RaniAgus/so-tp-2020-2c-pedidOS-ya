#include "csconsole.h"

char** cs_console_readline(const char* text, int* argc)
{
	char *line, **argv = NULL;

	line = readline(text);
	if(line[0] != '\0')
	{
		argv  = string_split(line, " ");
		*argc = cs_string_array_lines_count(argv);
	} else *argc = 0;
	free(line);

	return argv;
}
