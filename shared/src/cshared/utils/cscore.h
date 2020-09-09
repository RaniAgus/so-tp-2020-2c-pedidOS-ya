#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#define NON_NULL(...)	__attribute__((nonnull(__VA_ARGS__)))

#define ARGS(...) __VA_ARGS__

#define LAMBDA(return_type, args, code)\
	({ return_type __f__ args code __f__; })

//C Standard libs
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

//Unix libs
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <netdb.h>

//Readline
#include <readline/readline.h>

//UTN Commons
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/bitarray.h>



#endif /* UTILS_CORE_C_ */
