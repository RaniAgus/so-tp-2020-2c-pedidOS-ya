#ifndef CLPARSER_H_
#define CLPARSER_H_

#include <cshared/cshared.h>

typedef struct
{
	int8_t msgtype;
	t_consulta* msg;
}cl_parser_result;

typedef enum { CL_SUCCESS = 0, CL_ARGC_ERROR, CL_INVALID_MSGTYPE, CL_INVALID_ARG } cl_parser_error;

cl_parser_result* client_parse_arguments(int argc, char* argv[], e_module serv_module);

#endif
