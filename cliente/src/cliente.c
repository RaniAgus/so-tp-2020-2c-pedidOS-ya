#include "parser.h"

e_status client_send_msg(cl_parser_result result);

void cs_parse_argument(char* arg_with_dashes)
{
	char* arg = string_substring_from(arg_with_dashes,2);
	char** key_and_value = string_n_split(arg, 2, "=");

	if(!strcmp(key_and_value[0],"log-level"))
	{
		cs_logger_set_level(log_level_from_string(key_and_value[1]));
	}
	if(!strcmp(key_and_value[0],"run-tests"))
	{
		printf("Run tests\n");
	}

	string_iterate_lines(key_and_value,(void*) free);
	free(key_and_value);
	free(arg);
}

int main(int argc, char* argv[])
{
	e_status status = STATUS_SUCCESS;

#ifndef RELEASE
	for(int i = 1; i<argc; i++)
	{
		if(string_starts_with(argv[i],"--")) cs_parse_argument(argv[i]);
	}
#endif

	CHECK_STATUS(cs_config_init("../server/config.config"));
	CHECK_STATUS(cs_logger_init("CLIENT_LOGGER", "CLIENT"));

	CS_LOG_TRACE("Iniciado correctamente.");
	while(status == STATUS_SUCCESS)
	{
		cl_parser_status parser_status;
		cl_parser_result result;

		char **arg_values, *received;
		int arg_cant;

		received = readline("Ingrese mensaje a enviar (ENTER para finalizar):\n> ");
		if(!strcmp(received,""))
		{
			CS_LOG_TRACE("Se recibió un salto de línea.");
			free(received);
			break;
		}
		CS_LOG_TRACE("Se recibió la línea: %s", received);

		arg_values = string_split(received, " ");
		arg_cant   = cs_string_array_lines_count(arg_values);

		CS_LOG_TRACE("La cantidad de argumentos es: %d", arg_cant);

		parser_status = client_parse_arguments(&result, arg_cant, arg_values);
		if(parser_status == CL_SUCCESS)
		{
			CS_LOG_TRACE("Se parseó el mensaje: %s",
					cs_msg_to_str(result.msg, result.header.opcode, result.header.msgtype));

			status = client_send_msg(result);
		}
		else
		{
			client_print_parser_error(parser_status, result);
		}

		free(received);
	};

	cs_logger_delete();
	cs_config_delete();

	return status;
}

e_status client_send_msg(cl_parser_result result)
{
	e_status status;
	uint32_t ack;

	//Llama a la función que hace el envío del mensaje
	status = cs_connect_and_send_msg("IP", "PUERTO", result.header, result.msg, &ack);

	//No es necesario hacer siempre CHECK_STATUS, se puede chequear manualmente
	if(status == STATUS_SUCCESS)
	{
		CS_LOG_INFO("El mensaje fue recibido. [ID: %d]", ack);
	} else
	{
		CS_LOG_ERROR("%s#%d (" __FILE__ ":%s:%d) -- %s\n",
				 cs_enum_status_to_str(status), status, __func__ ,__LINE__, cs_string_error(status) );
	}

	//Libera recursos
	cs_msg_destroy(result.msg, result.header.opcode, result.header.msgtype);

	return status;
}
