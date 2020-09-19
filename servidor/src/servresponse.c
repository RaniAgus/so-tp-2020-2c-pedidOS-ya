#include "servresponse.h"

void server_send_rta_consultar_restaurantes(t_sfd client_conn)
{
	t_rta_cons_rest* respuesta;
	char* rta_to_str;
	t_header header = {OPCODE_RESPUESTA_OK, CONSULTAR_RESTAURANTES};
	respuesta = cs_rta_consultar_rest_create("[resto1,resto2,resto3]");
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_msg(client_conn, header, respuesta) == STATUS_SUCCESS )
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
	t_pos pos_restaurante = { 1 , 2 };

	respuesta = cs_rta_obtener_rest_create(
			5,
			"[Milanesa,Milanesa]",
			"[Milanesa,Empanada,Ensalada]",
			"[450,55,300]",
			pos_restaurante,
			2);

	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_msg(client_conn, header, respuesta) == STATUS_SUCCESS )
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
	respuesta = cs_rta_consultar_pl_create("[ravioles,nioquis,asado]");
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_msg(client_conn, header, respuesta) == STATUS_SUCCESS )
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

	if( cs_send_msg(client_conn, header, respuesta) == STATUS_SUCCESS )
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
	respuesta = cs_rta_consultar_ped_create("el crustaceo",PEDIDO_PENDIENTE,"[plato1,plato2]","[1,2]","[4,5]");
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);
	if( cs_send_msg(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);
}

void server_send_rta_obtener_pedido(t_sfd client_conn)
{
	t_rta_obt_ped* respuesta;
	char*rta_to_str;
	t_header header= {OPCODE_RESPUESTA_OK,OBTENER_PEDIDO};
	respuesta= cs_rta_obtener_ped_create("[milanga,asado]","[1,2]","[3,4]");
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_msg(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);
}

void server_send_rta_obtener_receta(t_sfd client_conn)
{
	t_rta_obt_rec* respuesta;
	char* rta_to_str;
	t_header header= {OPCODE_RESPUESTA_OK,OBTENER_RECETA};
	respuesta = cs_rta_obtener_receta_create("[Trocear,Empanar,Reposar,Hornear]","[4,5,3,10]");
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_msg(client_conn, header, respuesta) == STATUS_SUCCESS )
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
	if( cs_send_msg(client_conn, header, NULL) == STATUS_SUCCESS )
		{
			CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
		} else
		{
			CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
		}

		free(rta_to_str);
		cs_msg_destroy(NULL, header.opcode, header.msgtype);

}
