#ifndef CSHARED_UTILS_CSCONSOLE_H_
#define CSHARED_UTILS_CSCONSOLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "csstring.h"

/**
* @NAME cs_console_readline
* @DESC Lee una línea de stdin, agregándola al historial y
* devolviendo los argumentos en un array (junto con su cantidad)
*/
char** cs_console_readline(const char* text, int* argc);

/**
* @NAME cs_console_readline
* @DESC Almacena la línea que está siendo leida por readline
* (probablemente para loguear algo)
*/
void console_save_line(void);

/**
* @NAME console_restore_line
* @DESC Restaura la línea que fue almacenada (probablemente para
* loguear algo)
*/
void console_restore_line(void);

#endif /* CSHARED_UTILS_CSCONSOLE_H_ */
