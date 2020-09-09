#ifndef CLIENT_PARSER_H
#define CLIENT_PARSER_H

#include "../../shared/src/cshared.h"

typedef struct
{
	t_header header;
	void*	 msg;
}cl_parser_result;

typedef enum
{
	CL_SUCCESS = 0,
	CL_CANT_ARGS_ERROR,
	CL_MSGTYPE_ARG_ERROR,
	CL_ARGS_ERROR
}cl_parser_status;

cl_parser_status client_parse_arguments(cl_parser_result* result, int argc, char* argv[]);
void 			 client_print_parser_error(cl_parser_status status, cl_parser_result result);

#endif //CLIENT_PARSER_H
