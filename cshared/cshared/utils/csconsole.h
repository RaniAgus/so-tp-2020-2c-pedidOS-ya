#ifndef CSHARED_UTILS_CSCONSOLE_H_
#define CSHARED_UTILS_CSCONSOLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>
#include "csstring.h"

char** cs_console_readline(const char* text, int* argc);

void console_save_line(void);
void console_restore_line(void);

#endif /* CSHARED_UTILS_CSCONSOLE_H_ */
