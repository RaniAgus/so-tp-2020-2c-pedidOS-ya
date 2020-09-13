#include "clparser.h"

#define MODULE_NAME		 "CLIENTE"
#define CONFIG_FILE_PATH "cliente.config"
#define LOG_FILE_KEY	 "ARCHIVO_LOG"

char* serv_ip;
char* serv_port;
t_sfd serv_conn;

e_status client_init(pthread_t* thread_recv_msg);

void client_send_msg_routine(char* received);
void client_recv_msg_routine(void);

e_status client_send_msg(cl_parser_result* result);
e_status client_recv_msg(t_sfd conn, int8_t* msg_type);

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

	e_status status;
	pthread_t thread_recv_msg;

	status = client_init(&thread_recv_msg);

	while(status == STATUS_SUCCESS)
	{
		char* received;

		received = readline("Ingrese mensaje a enviar (ENTER para finalizar):\n> ");
		if(!strcmp(received,""))
		{
			CS_LOG_TRACE("Se recibió un salto de línea.");
			free(received);
			break;
		}
		CS_LOG_TRACE("Se recibió la línea: %s", received);

		client_send_msg_routine(received);
	}

	CS_LOG_TRACE("Finalizando...");

	pthread_cancel(thread_recv_msg);
	pthread_join(thread_recv_msg, NULL);
	close(serv_conn);

	cs_logger_delete();
	cs_config_delete();

	return status;
}

e_status client_init(pthread_t* thread_recv_msg)
{
	char *modulo, *serv_ip_key = string_new(), *serv_port_key = string_new();

	CHECK_STATUS(cs_config_init(CONFIG_FILE_PATH));
	CHECK_STATUS(cs_logger_init(LOG_FILE_KEY, MODULE_NAME));

	modulo = readline("Ingrese a qué módulo quiere conectarse:\n> ");

	string_append_with_format(&serv_ip_key, "IP_%s", modulo);
	string_append_with_format(&serv_port_key, "PUERTO_%s", modulo);

	serv_ip   = cs_config_get_string(serv_ip_key);
	serv_port = cs_config_get_string(serv_port_key);

	if(serv_ip == NULL || serv_port == NULL)
	{
		fprintf(stderr, "%s#%d (" __FILE__ ":%s:%d) -- Error al encontrar los keys para conectarse a %s\n",
				 cs_enum_status_to_str(STATUS_CONFIG_ERROR), STATUS_CONFIG_ERROR, __func__ ,__LINE__, modulo);
		exit(STATUS_CONFIG_ERROR);
	}

	free(serv_ip_key);
	free(serv_port_key);
	free(modulo);

	CHECK_STATUS(cs_tcp_client_create(&serv_conn, serv_ip, serv_port));
	CHECK_STATUS(PTHREAD_CREATE(thread_recv_msg, client_recv_msg_routine, NULL));

	CS_LOG_TRACE("Iniciado correctamente.");

	return STATUS_SUCCESS;
}

void client_send_msg_routine(char* received)
{
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
		pthread_t thread_msg;
		PTHREAD_CREATE(&thread_msg,client_send_msg, &result);
		pthread_detach(thread_msg);
	}
	else
	{
		client_print_parser_error(parser_status, result);
	}

	string_iterate_lines(arg_values, (void*) free);
	free(arg_values);
	free(received);

	return;
}

void client_recv_msg_routine(void)
{
	e_status status;
	do
	{
		t_header header;
		header.opcode = (int8_t)OPCODE_RESPUESTA_OK;

		status = client_recv_msg(serv_conn, &header.msgtype);
		if(status == STATUS_SUCCESS)
		{
			status = cs_send_msg(serv_conn, header, NULL);
		}
	} while(status == STATUS_SUCCESS);
}

e_status client_send_msg(cl_parser_result* result)
{
	e_status status;
	t_sfd conn;

	//Se conecta al servidor
	status = cs_tcp_client_create(&conn, serv_ip, serv_port);
	if(status == STATUS_SUCCESS)
	{
		//Envía el mensaje
		status = cs_send_msg(conn, result->header, result->msg);
		if(status == STATUS_SUCCESS)
		{
			CS_LOG_INFO("Mensaje enviado: %s",
					cs_msg_to_str(result->msg, result->header.opcode, result->header.msgtype));

			//Espera la respuesta
			status = client_recv_msg(conn, NULL);
		}
	}
	if(status != STATUS_SUCCESS)
	{
		printf("\n");
		fprintf(stderr, "%s#%d (" __FILE__ ":%s:%d) -- %s\n",
				 cs_enum_status_to_str(status), status, __func__ ,__LINE__, cs_string_error(status) );
		printf("> ");
	}

	//Libera recursos
	cs_msg_destroy(result->msg, result->header.opcode, result->header.msgtype);

	return status;
}

e_status client_recv_msg(t_sfd conn, int8_t* msg_type)
{
	void _recv_and_print_msg(t_sfd conn, t_header header, void* msg)
	{
		char* msg_str;

		msg_str = cs_msg_to_str(msg, header.opcode, header.msgtype);
		printf("\n");
		CS_LOG_INFO("Mensaje recibido: %s", cs_msg_to_str(msg, header.opcode, header.msgtype));
		printf("> ");

		free(msg_str);
		cs_msg_destroy(msg, header.opcode, header.msgtype);

		if(msg_type) *msg_type = (int8_t)header.msgtype;
	}

	return cs_recv_msg(conn, _recv_and_print_msg);
}
