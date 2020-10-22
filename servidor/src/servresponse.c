#include "servresponse.h"

void server_send_rta_handshake_cli(t_sfd client_conn)
{
	t_rta_handshake_cli* respuesta;
	char* rta_to_str;
	t_header header = {OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE};
	respuesta = cs_rta_handshake_cli_create();
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_respuesta(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);
}

void server_send_rta_consultar_restaurantes(t_sfd client_conn)
{
	t_rta_cons_rest* respuesta;
	char* rta_to_str;
	t_header header = {OPCODE_RESPUESTA_OK, CONSULTAR_RESTAURANTES};
	respuesta = cs_rta_consultar_rest_create(string_get_string_as_array("[resto1,resto2,resto3]"));
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_respuesta(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);
}

void server_send_rta_obtener_restaurante(t_sfd client_conn)
{
	t_rta_obt_rest* respuesta;
	char* rta_to_str;

	t_header header = { OPCODE_RESPUESTA_OK, OBTENER_RESTAURANTE };
	t_pos pos_restaurante = { 5 , 5 };

	respuesta = cs_rta_obtener_rest_create(
			3,
			"[AsadoCompleto]",
			"[AsadoCompleto, Choripan]",
			"[180,100]",
			pos_restaurante,
			1,
			0
	);

	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_respuesta(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);
}

void server_send_rta_consultar_platos(t_sfd client_conn)
{

	t_rta_cons_pl* respuesta;
	char* rta_to_str;
	t_header header = {OPCODE_RESPUESTA_OK, CONSULTAR_PLATOS};
	respuesta = cs_rta_consultar_pl_create("[AsadoCompleto, Choripan]");
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_respuesta(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);

}

void server_send_rta_crear_pedido(t_sfd client_conn)
{
	t_rta_crear_ped* respuesta;
	char* rta_to_str;
	t_header header = {OPCODE_RESPUESTA_OK, CREAR_PEDIDO};
	respuesta = cs_rta_crear_ped_create(27);
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_respuesta(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);
}

void server_send_rta_consultar_pedido(t_sfd client_conn)
{
	t_rta_cons_ped* respuesta;
	char* rta_to_str;
	t_header header = {OPCODE_RESPUESTA_OK, CONSULTAR_PEDIDO};
	respuesta = cs_rta_consultar_ped_create("ElParrillon",PEDIDO_CONFIRMADO,"[AsadoCompleto,Choripan]","[0,0]","[1,2]");
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);
	if( cs_send_respuesta(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);
}

void server_send_rta_obtener_pedido(t_sfd client_conn, uint32_t id)
{
	t_rta_obt_ped* respuesta;
	char*rta_to_str;
	t_header header= {OPCODE_RESPUESTA_OK,OBTENER_PEDIDO};
	if(id == 1) {
		respuesta= cs_rta_obtener_ped_create(PEDIDO_CONFIRMADO, "[AsadoCompleto,Choripan]","[0,0]","[1,2]");
	} else if (id == 2) {
		respuesta= cs_rta_obtener_ped_create(PEDIDO_CONFIRMADO, "[Choripan]","[0]","[5]");
	} else {
		respuesta= cs_rta_obtener_ped_create(PEDIDO_CONFIRMADO, "[milanga,asado]","[1,2]","[3,4]");
	}
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_respuesta(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);
}

void server_send_rta_obtener_receta(t_sfd client_conn, char* comida)
{
	t_rta_obt_rec* respuesta;
	char* rta_to_str;
	t_header header = {OPCODE_RESPUESTA_OK,OBTENER_RECETA};
	if(!strcmp(comida, "AsadoCompleto"))
		respuesta = cs_rta_obtener_receta_create("[Preparar,Servir]","[10,2]");
	else if(!strcmp(comida, "Choripan"))
		respuesta = cs_rta_obtener_receta_create("[Preparar,Servir]","[2,1]");
	else
		respuesta = cs_rta_obtener_receta_create("[Trocear,Empanar,Reposar,Hornear]","[4,5,3,10]");

	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_respuesta(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);
}


void server_send_rta_ok(e_msgtype msg_type, t_sfd client_conn)
{
	t_header header= {OPCODE_RESPUESTA_OK,msg_type};
	char* rta_to_str = cs_msg_to_str(NULL, header.opcode, header.msgtype);
	if( cs_send_respuesta(client_conn, header, NULL) == STATUS_SUCCESS )
		{
			CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
		} else
		{
			CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
		}

		free(rta_to_str);
		cs_msg_destroy(NULL, header.opcode, header.msgtype);

}
