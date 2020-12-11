#include "clmessages.h"

e_status client_send_handshake(t_sfd serv_conn, int8_t* module)
{
	e_status status;

	status = cs_send_handshake_cli(serv_conn);
	if (status == STATUS_SUCCESS) {
		status = client_recv_msg(serv_conn, NULL, module);
	}

	return status;
}

e_status client_recv_msg(t_sfd conn, int8_t* msg_type, int8_t* module)
{
	void _recv_and_print_msg(t_sfd conn, t_header header, void* msg) {
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

	CS_LOG_INFO("Se perdió la conexión con %s.", cs_enum_module_to_str(serv_module));
}

e_status client_send_msg(cl_parser_result* result)
{
	e_status status;
	t_sfd conn;
	char* msg_to_str = cs_msg_to_str(result->msg, OPCODE_CONSULTA, result->msgtype);

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
				CS_LOG_INFO("Mensaje enviado: %s", msg_to_str);
				//Espera la respuesta
				status = client_recv_msg(conn, NULL, NULL);
			}
		}
	}
	if(status != STATUS_SUCCESS) CS_LOG_ERROR("%s -- No se pudo enviar el mensaje %s", cs_enum_status_to_str(status), msg_to_str);

	free(msg_to_str);
	close(conn);
	cs_msg_destroy(result->msg, OPCODE_CONSULTA, result->msgtype);
	free(result);

	return status;
}
