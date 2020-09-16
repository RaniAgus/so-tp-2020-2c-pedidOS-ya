#include "csconsole.h"

char** _get_tokens_array(char* line, int* argc);

char** cs_console_readline(const char* text, int* argc)
{
	char *line, **argv = NULL;
	*argc = 0;

	line = readline(text);
	if(line[0] != '\0')
	{
		add_history(line);
		argv = _get_tokens_array(line, argc);
	}
	free(line);

	return argv;
}

char** _get_tokens_array(char* line, int* argc)
{
	char *buff, *token, **argv = NULL;

	buff  = line;
	token = strtok_r(line, " ", &buff);

	if(token != NULL)
	{
		(*argc)++;
		argv = realloc(argv, sizeof(char*) * (*argc));
		argv[(*argc) - 1] = string_duplicate(token);

		while(buff[0] != '\0')
		{
			token = strtok_r(NULL, " ", &buff);
			if(token == NULL) break;

			(*argc)++;
			argv = realloc(argv, sizeof(char*) * (*argc));
			argv[(*argc) - 1] = string_duplicate(token);
		}
	}

	argv = realloc(argv, sizeof(char*) * ((*argc) + 1));
	argv[(*argc)] = NULL;

	return argv;
}
