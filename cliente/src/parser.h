#ifndef CLIENT_PARSER_H
#define CLIENT_PARSER_H

#include "../../shared/src/cshared.h"

typedef struct
{
	t_header header;
	void*	 msg;
}cl_parser_result;

#define CL_SUCCESS			0

int client_parse_arguments(cl_parser_result* result, int argc, char* argv[]);
void client_print_parser_error(int status, cl_parser_result result);

#endif //CLIENT_PARSER_H
