#include "clparser.h"

#define MODULE_NAME		 "CLIENTE"
#define CONFIG_FILE_PATH "cliente.config"
#define LOG_FILE_KEY	 "ARCHIVO_LOG"
#define IP_SERVER 		 "IP"
#define PORT_SERVER 	 "PUERTO"

char* serv_ip;
char* serv_port;
t_sfd serv_conn;

e_status client_init(pthread_t* thread_recv_msg);

void client_recv_msg_routine(void);
e_status client_send_msg(cl_parser_result* result);
e_status client_recv_msg(t_sfd conn, int8_t* msg_type);

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
		int    arg_cant;
		char** arg_values;

		cl_parser_status  parser_status;
		cl_parser_result *result = malloc(sizeof(cl_parser_result));

		arg_values = cs_console_readline("Ingrese mensaje a enviar (ENTER para finalizar):\n> ", &arg_cant);
		if(arg_values == NULL)
		{
			CS_LOG_TRACE("Se recibió un salto de línea.");
			free(result);
			break;
		}

		//Parsea los argumentos
		parser_status = client_parse_arguments(result, arg_cant, arg_values);
		if(parser_status == CL_SUCCESS)
		{
			char* msg_to_str = cs_msg_to_str(result->msg, result->header.opcode, result->header.msgtype);
			CS_LOG_TRACE("Se parseó el mensaje: %s", msg_to_str);
			free(msg_to_str);

			//Envía el mensaje
			pthread_t thread_msg;
			PTHREAD_CREATE(&thread_msg, client_send_msg, result);
			pthread_detach(thread_msg);
		}
		else
		{
			client_print_parser_error(parser_status, *result);
			free(result);
		}

		string_iterate_lines(arg_values, (void*) free);
		free(arg_values);
	}

	CS_LOG_TRACE("Finalizando...");

	pthread_cancel(thread_recv_msg);
	pthread_join(thread_recv_msg, NULL);
	close(serv_conn);

	cs_module_close();

	return status;
}

e_status client_init(pthread_t* thread_recv_msg)
{
	//Inicia los config y logger
	cs_module_init(CONFIG_FILE_PATH, LOG_FILE_KEY, MODULE_NAME);

	//serv_ip y serv_port almacenan la info del servidor a conectarse
	serv_ip   = cs_config_get_string(IP_SERVER);
	serv_port = cs_config_get_string(PORT_SERVER);

	if(serv_ip == NULL || serv_port == NULL)
	{
		PRINT_ERROR(STATUS_CONFIG_ERROR);
		exit(STATUS_CONFIG_ERROR);
	}

	//Crea el primer socket para recibir mensajes ahí
	CHECK_STATUS(cs_tcp_client_create(&serv_conn, serv_ip, serv_port));
	CHECK_STATUS(PTHREAD_CREATE(thread_recv_msg, client_recv_msg_routine, NULL));

	CS_LOG_TRACE("Iniciado correctamente.");

	return STATUS_SUCCESS;
}

e_status client_send_handshake(t_sfd serv_conn)
{
	e_status status;
	//Envia el hs
	t_header header = { OPCODE_CONSULTA, HANDSHAKE };

	t_handshake* msg = cs_cons_handshake_create(
			cs_config_get_string("ID_CLIENTE"),
			(uint32_t) cs_config_get_int("POSICION_X"),
			(uint32_t) cs_config_get_int("POSICION_Y"));

	status = cs_send_msg(serv_conn, header, (void*) msg);
	if (status == STATUS_SUCCESS)
	{
		status = client_recv_msg(serv_conn, NULL);
	}
	return status;
}

void client_recv_msg_routine(void)
{
	e_status status = client_send_handshake(serv_conn);
	while(status == STATUS_SUCCESS)
	{
		t_header header;

		//Recibe la consulta
		status = client_recv_msg(serv_conn, &header.msgtype);
		if(status == STATUS_SUCCESS)
		{
			//Cliente solo recibe "Terminar Pedido"
			if(header.msgtype == TERMINAR_PEDIDO || header.msgtype == PLATO_LISTO)
			{
				header.opcode = (int8_t)OPCODE_RESPUESTA_OK;
			} else
			{
				header.opcode = (int8_t)OPCODE_RESPUESTA_FAIL;
			}
			//Envía la respuesta
			status = cs_send_msg(serv_conn, header, NULL);
		}
	}

	fprintf(stderr, "%s#%d (" __FILE__ ":%s:%d) -- %s\n",
			 cs_enum_status_to_str(status), status, __func__ ,__LINE__, cs_string_error(status) );
}

e_status client_send_msg(cl_parser_result* result)
{
	e_status status;
	t_sfd conn;

	//Se conecta al servidor
	status = cs_tcp_client_create(&conn, serv_ip, serv_port);
	if(status == STATUS_SUCCESS)
	{
		//Envía el hs
		status = client_send_handshake(conn);
		if(status == STATUS_SUCCESS)
		{
			//Envía el mensaje
			status = cs_send_msg(conn, result->header, result->msg);
			if(status == STATUS_SUCCESS)
			{
				char* msg_to_str = cs_msg_to_str(result->msg, result->header.opcode, result->header.msgtype);
				CS_LOG_INFO("Mensaje enviado: %s", msg_to_str);
				free(msg_to_str);

				//Espera la respuesta
				status = client_recv_msg(conn, NULL);
			}
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
	free(result);

	return status;
}

e_status client_recv_msg(t_sfd conn, int8_t* msg_type)
{
	void _recv_and_print_msg(t_sfd conn, t_header header, void* msg)
	{
		char* msg_str;

		msg_str = cs_msg_to_str(msg, header.opcode, header.msgtype);
		printf("\n");
		CS_LOG_INFO("Mensaje recibido: %s", msg_str);
		printf("> ");

		free(msg_str);
		cs_msg_destroy(msg, header.opcode, header.msgtype);

		if(msg_type) *msg_type = header.msgtype;
	}

	return cs_recv_msg(conn, _recv_and_print_msg);
}
