#include "clparser.h"

#define CONFIG_FILE_PATH "cliente.config"
#define LOG_FILE_KEY	 "ARCHIVO_LOG"
#define IP_SERVER 		 "IP"
#define PORT_SERVER 	 "PUERTO"

char*  prompt;
char*  serv_ip;
char*  serv_port;
t_sfd  serv_conn;
int8_t serv_module;

e_status client_send_handshake(t_sfd serv_conn, int8_t* module);
void client_recv_msg_routine(void);
e_status client_send_msg(cl_parser_result* result);
e_status client_recv_msg(t_sfd conn, int8_t* msg_type, int8_t* module);

e_status client_init(pthread_t* thread_recv_msg)
{
	char* module_name;

	//Inicia los config y logger
	CHECK_STATUS(cs_config_init(CONFIG_FILE_PATH));
	module_name = strdup(cs_config_get_string("ID_CLIENTE"));
	string_to_upper(module_name);
	CHECK_STATUS(cs_logger_init(LOG_FILE_KEY, module_name));

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
	CHECK_STATUS(client_send_handshake(serv_conn, &serv_module));
	pthread_create(thread_recv_msg, NULL, (void*) client_recv_msg_routine, NULL);

	CS_LOG_TRACE(__FILE__":%s:%d -- Iniciado correctamente.", __func__, __LINE__);
	
	free(module_name);
	return STATUS_SUCCESS;
}

int main(int argc, char* argv[])
{
	for(int i = 1; i<argc; i++)
	{
		if(string_starts_with(argv[i],"-")) cs_parse_argument(argv[i]);
	}

	e_status status;
	pthread_t thread_recv_msg;

	status = client_init(&thread_recv_msg);

	while(status == STATUS_SUCCESS)
	{
		int    arg_cant;
		char** arg_values;

		cl_parser_status  parser_status;
		cl_parser_result *result;

		arg_values = cs_console_readline(prompt, &arg_cant);
		if(arg_values == NULL)
		{
			continue;
		} else if(!strcmp(arg_values[0], "exit")) {
			CS_LOG_TRACE(__FILE__":%s:%d -- Se recibió el comando exit.", __func__, __LINE__);
			string_iterate_lines(arg_values, (void*) free);
			free(arg_values);
			break;
		}
		result = malloc(sizeof(cl_parser_result));

		//Parsea los argumentos
		parser_status = client_parse_arguments(result, arg_cant, arg_values, serv_module);
		if(parser_status == CL_SUCCESS)
		{
			char* msg_to_str = cs_msg_to_str(result->msg, OPCODE_CONSULTA, result->msgtype);
			CS_LOG_TRACE(__FILE__":%s:%d -- Se parseó el mensaje: %s",  __func__, __LINE__, msg_to_str);
			free(msg_to_str);

			//Envía el mensaje
			pthread_t thread_msg;
			pthread_create(&thread_msg, NULL, (void*) client_send_msg, (void*) result);
			pthread_detach(thread_msg);
		}
		else
		{
			client_print_parser_error(parser_status, *result, serv_module);
			free(result);
		}

		string_iterate_lines(arg_values, (void*) free);
		free(arg_values);
	}

	CS_LOG_TRACE(__FILE__":%s:%d -- Finalizando...", __func__, __LINE__);

	pthread_cancel(thread_recv_msg);
	pthread_join(thread_recv_msg, NULL);
	close(serv_conn);
	free(prompt);

	cs_module_close();

	return status;
}

e_status client_send_handshake(t_sfd serv_conn, int8_t* module)
{
	e_status status;

	status = cs_send_handshake_cli(serv_conn);
	if (status == STATUS_SUCCESS)
	{
		status = client_recv_msg(serv_conn, NULL, module);
	}
	
	return status;
}

void client_recv_msg_routine(void)
{
	e_status status;
	do
	{
		t_header header;

		//Recibe la consulta
		status = client_recv_msg(serv_conn, &header.msgtype, NULL);
		if(status == STATUS_SUCCESS)
		{
			//Cliente solo recibe "Terminar Pedido"
			if(header.msgtype == FINALIZAR_PEDIDO || header.msgtype == PLATO_LISTO)
			{
				header.opcode = (int8_t)OPCODE_RESPUESTA_OK;
			} else
			{
				header.opcode = (int8_t)OPCODE_RESPUESTA_FAIL;
			}
			//Envía la respuesta
			status = cs_send_respuesta(serv_conn, header, NULL);
			
			char* rta_str = cs_msg_to_str(NULL, header.opcode, header.msgtype);
			if(status == STATUS_SUCCESS) {
				CS_LOG_INFO("Se envió la respuesta: %s", rta_str);
			} else {
				CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_str);
			}
			free(rta_str);
			
		}
	} while(status == STATUS_SUCCESS);

	system("reset -Q");
	CS_LOG_INFO("Se perdió la conexión con %s.", cs_enum_module_to_str(serv_module));
	console_save_line();
	exit(-1);
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
		status = client_send_handshake(conn, NULL);
		if(status == STATUS_SUCCESS)
		{
			//Envía el mensaje
			status = cs_send_consulta(conn, result->msgtype, result->msg, serv_module);
			if(status == STATUS_SUCCESS)
			{
				char* msg_to_str = cs_msg_to_str(result->msg, OPCODE_CONSULTA, result->msgtype);
				CS_LOG_INFO("Mensaje enviado: %s", msg_to_str);
				free(msg_to_str);

				//Espera la respuesta
				status = client_recv_msg(conn, NULL, NULL);
			}
		}
	}
	if(status != STATUS_SUCCESS) PRINT_ERROR(status);

	close(conn);
	cs_msg_destroy(result->msg, OPCODE_CONSULTA, result->msgtype);
	free(result);

	return status;
}

e_status client_recv_msg(t_sfd conn, int8_t* msg_type, int8_t* module)
{
	void _recv_and_print_msg(t_sfd conn, t_header header, void* msg)
	{
		char* msg_str = cs_msg_to_str(msg, header.opcode, header.msgtype);

		if(header.msgtype != HANDSHAKE_CLIENTE) {
			CS_LOG_INFO("Mensaje recibido: %s", msg_str);
		} else {
			CS_LOG_TRACE(__FILE__":%s:%d -- Mensaje recibido: %s",  __func__, __LINE__, msg_str);
		}
		if(module) {
			prompt = string_from_format("PedidOS Ya!:~/%s$ ", cs_enum_module_to_str(RTA_HANDSHAKE_PTR(msg)->modulo));
			*module = RTA_HANDSHAKE_PTR(msg)->modulo;
		}
		if(msg_type) {
			*msg_type = header.msgtype;
		}

		free(msg_str);
		cs_msg_destroy(msg, header.opcode, header.msgtype);
	}

	return cs_recv_msg(conn, _recv_and_print_msg);
}
