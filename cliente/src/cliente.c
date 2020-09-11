#include "clparser.h"

t_sfd conn;

e_status client_send_msg_routine(char* received);
e_status client_send_msg(cl_parser_result result);
void 	 client_recv_msg_routine(t_sfd* conn);

void cs_parse_argument(char* arg)
{
	char *arg_without_dashes, **key_and_value;

	arg_without_dashes = string_substring_from(arg, 2);
	key_and_value = string_n_split(arg_without_dashes, 2, "=");

	if(!strcmp(key_and_value[0],"log-level"))
	{
		cs_logger_set_level(log_level_from_string(key_and_value[1]));
	}

	string_iterate_lines(key_and_value,(void*) free);
	free(key_and_value);
	free(arg_without_dashes);
}

int main(int argc, char* argv[])
{

	for(int i = 1; i<argc; i++)
	{
		if(string_starts_with(argv[i],"--")) cs_parse_argument(argv[i]);
	}

	CHECK_STATUS(cs_config_init("cliente.config"));
	CHECK_STATUS(cs_logger_init("ARCHIVO_LOG", "CLIENTE"));

	e_status status = STATUS_SUCCESS;;

	//TODO: Seleccionar módulo y conectarse
	//TODO: Ver cómo recibir los mensajes
	conn = -1;

	CS_LOG_TRACE("Iniciado correctamente.");
	while(status == STATUS_SUCCESS)
	{
		char *received;

		received = readline("Ingrese mensaje a enviar (ENTER para finalizar):\n> ");
		if(!strcmp(received,""))
		{
			CS_LOG_TRACE("Se recibió un salto de línea.");
			free(received);
			break;
		}
		CS_LOG_TRACE("Se recibió la línea: %s", received);

		status = client_send_msg_routine(received);
	}

	CS_LOG_TRACE("Finalizando...");
	cs_logger_delete();
	cs_config_delete();

	return status;
}

e_status client_send_msg_routine(char* received)
{
	e_status status = STATUS_SUCCESS;

	cl_parser_status parser_status;
	cl_parser_result result;

	//Divide los argumentos separados por espacios
	char** arg_values = string_split(received, " ");
	int    arg_cant   = cs_string_array_lines_count(arg_values);

	CS_LOG_TRACE("La cantidad de argumentos es: %d", arg_cant);

	//Parsea los argumentos
	parser_status = client_parse_arguments(&result, arg_cant, arg_values);
	if(parser_status == CL_SUCCESS)
	{
		char* msg_to_str = cs_msg_to_str(result.msg, result.header.opcode, result.header.msgtype);
		CS_LOG_TRACE("Se parseó el mensaje: %s", msg_to_str);
		free(msg_to_str);

		//Envía el mensaje
		status = client_send_msg(result);
	}
	else
	{
		client_print_parser_error(parser_status, result);
	}

	string_iterate_lines(arg_values, (void*) free);
	free(arg_values);
	free(received);

	return status;
}

e_status client_send_msg(cl_parser_result result)
{
	e_status status;

	//Llama a la función que hace el envío del mensaje
	status = cs_send_msg(conn, result.header, result.msg);

	//Si se envió correctamente, espera la respuesta
	if(status == STATUS_SUCCESS)
	{
		CS_LOG_TRACE("Se envió el mensaje correctamente.");

		pthread_t msg_thread;
		CHECK_STATUS(PTHREAD_CREATE(msg_thread, client_send_msg, NULL));
		pthread_detach(msg_thread);
	}
	if(status != STATUS_SUCCESS)
	{
		CS_LOG_ERROR("%s#%d (" __FILE__ ":%s:%d) -- %s\n",
				 cs_enum_status_to_str(status), status, __func__ ,__LINE__, cs_string_error(status) );
	}

	//Libera recursos
	cs_msg_destroy(result.msg, result.header.opcode, result.header.msgtype);

	return status;
}

void client_recv_msg_routine(t_sfd* conn)
{
	/*
	e_status status = STATUS_SUCCESS;

	void _recv_and_print_msg(t_header header, void* msg)
	{
		char* msg_str;

		msg_str = cs_msg_to_str(msg, header.opcode, header.msgtype);
		printf("\n");
		CS_LOG_INFO("Mensaje recibido: %s", cs_msg_to_str(msg, header.opcode, header.msgtype));
		printf("> ");
		free(msg_str);
		cs_msg_destroy(msg, header.opcode, header.msgtype);
	}

	while(conn != -1)
		status = cs_recv_msg(???, _recv_and_print_msg);
 */
}
