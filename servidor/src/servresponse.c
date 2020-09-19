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

	//TODO: server_send_rta_consultar_platos
	t_rta_cons_pl* respuesta;
	char* rta_to_str;
	t_header header = {OPCODE_RESPUESTA_OK, CONSULTAR_PLATOS};
	respuesta = cs_rta_consultar_pl_create("[{comida1,cantTotal1,cantLista1},{comida2,cantTotal2,cantLista2}]");
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
	//TODO: server_send_rta_consultar_pedido
}

void server_send_rta_obtener_pedido(t_sfd client_conn)
{
	//TODO: server_send_rta_obtener_pedido
}

void server_send_rta_consultar_receta(t_sfd client_conn)
{
	//TODO: server_send_rta_consultar_receta
}


void server_send_rta_ok(e_msgtype msg_type, t_sfd client_conn)
{
	//TODO: server_send_rta_ok
}
