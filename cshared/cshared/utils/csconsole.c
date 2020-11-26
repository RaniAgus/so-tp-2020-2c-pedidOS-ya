#include "csconsole.h"

static char *saved_line;
static int  saved_point;
static int need_hack;

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

void console_save_line(void)
{
    need_hack = (rl_readline_state & RL_STATE_READCMD) > 0;
    if(need_hack)
    {
        saved_point = rl_point;
        saved_line = rl_copy_text(0, rl_end);
        rl_save_prompt();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

void console_restore_line(void)
{
    if(need_hack)
    {
        rl_restore_prompt();
        rl_replace_line(saved_line, 0);
        rl_point = saved_point;
        rl_redisplay();
        free(saved_line);
    }
}

char** _get_tokens_array(char* line, int* argc)
{
	char *buff, *token, **argv = NULL;

	buff  = line;
	token = strtok_r(line, " ", &buff);

	if(token != NULL)
	{
		argv = string_array_new();

		(*argc)++;
		string_array_push(&argv, string_duplicate(token));

		while(buff[0] != '\0')
		{
			token = strtok_r(NULL, " ", &buff);
			if(token == NULL) break;

			(*argc)++;
			string_array_push(&argv, string_duplicate(token));
		}
	}

	return argv;
}
