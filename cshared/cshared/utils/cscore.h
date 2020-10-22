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
#include <netdb.h>

#endif /* UTILS_CORE_C_ */
