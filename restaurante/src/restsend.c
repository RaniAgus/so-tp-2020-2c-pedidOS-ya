#include "restsend.h"

static void* rest_enviar_consulta(e_module dest, t_sfd conexion_comanda, int8_t msg_type, t_consulta* consulta, int8_t* result);

t_rta_obt_rest* rest_obtener_metadata(void)
{
	int8_t result;
	t_consulta* consulta;
	t_rta_obt_rest* metadata;

	consulta = cs_msg_obtener_rest_create(mi_nombre);
	metadata = rest_consultar_sindicato(OBTENER_RESTAURANTE, consulta, &result);
	cs_msg_destroy(consulta, OPCODE_CONSULTA, OBTENER_RESTAURANTE);

	if(result != OPCODE_RESPUESTA_OK) {
		CS_LOG_ERROR("No se pudo obtener la metadata de %s", mi_nombre);
		exit(-1);
	}

	return metadata;
}

t_rta_obt_ped* rest_obtener_pedido(uint32_t pedido_id, int8_t* result)
{
	t_consulta* cons = cs_msg_obtener_ped_create(mi_nombre, pedido_id);
	t_rta_obt_ped* pedido = rest_consultar_sindicato(OBTENER_PEDIDO, cons, result);
	cs_msg_destroy(cons, OPCODE_CONSULTA, OBTENER_PEDIDO);

	return pedido;
}

void* rest_consultar_sindicato(int8_t msg_type, t_consulta* consulta, int8_t* result)
{
	e_status status;
	t_sfd conexion_sindicato;

	//Se conecta como cliente
	status = cs_tcp_client_create(&conexion_sindicato,
			cs_config_get_string("IP_SINDICATO"),
			cs_config_get_string("PUERTO_SINDICATO")
	);
	if(status != STATUS_SUCCESS)
	{
		CS_LOG_ERROR("%s -- No se pudo conectar con Comanda. Finalizando.",
				cs_enum_status_to_str(status)
		);
		exit(-1);
	}
	CS_LOG_TRACE("Conectado exitosamente con Sindicato.");

	return rest_enviar_consulta(MODULO_SINDICATO, conexion_sindicato, msg_type, consulta, result);
}

static void* rest_enviar_consulta(e_module dest, t_sfd conexion_comanda, int8_t msg_type, t_consulta* consulta, int8_t* result)
{
	e_status status;
	void* rta;
	char* consulta_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, msg_type);

	//Envía la consulta
	status = cs_send_consulta(conexion_comanda, msg_type, consulta, dest);
	if(status == STATUS_SUCCESS)
	{
		CS_LOG_TRACE("Se envió la consulta: %s", consulta_str);

		//Recibe la respuesta
		void _recibir_respuesta(t_sfd conexion, t_header header_recibido, void* respuesta)
		{
			char* rta_str = cs_msg_to_str(respuesta, header_recibido.opcode,  header_recibido.msgtype);
			CS_LOG_TRACE("Se recibió la respuesta: %s", rta_str);
			free(rta_str);

			//Guarda la respuesta para retornarla
			if( header_recibido.msgtype == msg_type )
			{
				*result = header_recibido.opcode;
				rta = respuesta;
			}
			else
			{
				*result = OPCODE_RESPUESTA_FAIL;
				rta = NULL;
				CS_LOG_ERROR("eso no me lo esperaba");
			}
		}
		status = cs_recv_msg(conexion_comanda, _recibir_respuesta);
		if(status != STATUS_SUCCESS)
		{
			*result = OPCODE_RESPUESTA_FAIL;
			rta = NULL;
			CS_LOG_ERROR("%s -- No se pudo recibir la respuesta a: %s",
					cs_enum_status_to_str(status),
					consulta_str
			);
		}
	}
	else
	{
		*result = OPCODE_RESPUESTA_FAIL;
		rta = NULL;
		CS_LOG_ERROR("%s -- No se pudo enviar la consulta: %s",
				cs_enum_status_to_str(status),
				consulta_str
		);
	}
	free(consulta_str);

	return rta;
}
