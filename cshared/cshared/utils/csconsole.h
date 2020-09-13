#ifndef CSHARED_UTILS_CSCONSOLE_H_
#define CSHARED_UTILS_CSCONSOLE_H_

#include <stdlib.h>
#include <readline/readline.h>
#include <commons/string.h>
#include "csstring.h"

char** cs_console_readline(const char* text, int* argc);

#endif /* CSHARED_UTILS_CSCONSOLE_H_ */
