#include "restenvio.h"

static t_sfd conexion_app;

static int8_t rest_terminar_pedido_si_corresponde(uint32_t pedido_id);

static void* rest_enviar_consulta(e_module dest, t_sfd conexion, int8_t msg_type, t_consulta* consulta, int8_t* result);
static void* rest_recibir_respuesta(e_module src, t_sfd conexion, int8_t msg_type, int8_t* result);

t_rta_obt_rest* rest_obtener_metadata(void)
{
	int8_t result;
	t_rta_obt_rest* metadata;
	t_consulta* consulta;

	consulta = cs_msg_obtener_rest_create(mi_nombre);
	metadata = rest_consultar_sindicato(OBTENER_RESTAURANTE, consulta, &result);
	if(result != OPCODE_RESPUESTA_OK)
	{
		CS_LOG_ERROR("No se pudo obtener la metadata de %s", mi_nombre);
		exit(-1);
	}
	cs_msg_destroy(consulta, OPCODE_CONSULTA, OBTENER_RESTAURANTE);

	return metadata;
}

e_status rest_crear_conexion_app(t_sfd* conn)
{
	e_status status = cs_tcp_client_create(conn, cs_config_get_string("IP_APP"), cs_config_get_string("PUERTO_APP"));
	if(status != STATUS_SUCCESS)
	{
		CS_LOG_WARNING("%s -- No se pudo establecer conexión con App.", cs_enum_status_to_str(status));
	}

	return status;
}

void rest_conectarse_app(void)
{
	if(rest_crear_conexion_app(&conexion_app) == STATUS_SUCCESS)
	{
		e_status status = cs_send_handshake_res(conexion_app, mi_posicion);
		if(status == STATUS_SUCCESS)
		{
			void _recibir_handshake(t_sfd conn, t_header header, void* msg) {
				if(header.opcode == OPCODE_RESPUESTA_OK && header.msgtype == HANDSHAKE_RESTAURANTE) {
					CS_LOG_INFO("Se estableció conexión con App.");
				} else {
					printf("a veces cuando planeas una cosa, te sale otra completamente diferente");
				}
			}
			status = cs_recv_msg(conexion_app, _recibir_handshake);
		}

		if(status != STATUS_SUCCESS)
		{
			CS_LOG_WARNING("%s -- No se pudo establecer conexión con App.", cs_enum_status_to_str(status));
		}
	}
}

void* rest_consultar_sindicato(int8_t msg_type, t_consulta* consulta, int8_t* result)
{
	e_status status;
	t_sfd conexion_sindicato;
	void* respuesta;

	//Se conecta como cliente
	status = cs_tcp_client_create(&conexion_sindicato, cs_config_get_string("IP_SINDICATO"), cs_config_get_string("PUERTO_SINDICATO"));
	if(status != STATUS_SUCCESS)
	{
		CS_LOG_ERROR("%s -- No se pudo consultar con Sindicato.", cs_enum_status_to_str(status));
		*result = OPCODE_RESPUESTA_FAIL;
		return NULL;
	}
	CS_LOG_TRACE("Conectado exitosamente con Sindicato.");

	respuesta = rest_enviar_consulta(MODULO_SINDICATO, conexion_sindicato, msg_type, consulta, result);
	close(conexion_sindicato);

	return respuesta;
}

t_rta_obt_ped* rest_obtener_pedido(uint32_t pedido_id, int8_t* result)
{
	t_consulta* cons = cs_msg_obtener_ped_create(mi_nombre, pedido_id);
	t_rta_obt_ped* pedido = rest_consultar_sindicato(OBTENER_PEDIDO, cons, result);
	cs_msg_destroy(cons, OPCODE_CONSULTA, OBTENER_PEDIDO);

	return pedido;
}

t_rta_obt_rec* rest_obtener_receta(char* comida, int8_t* result)
{
	t_consulta* cons = cs_msg_rta_obtener_receta_create(comida);
	t_rta_obt_rec* receta = rest_consultar_sindicato(OBTENER_RECETA, cons, result);
	cs_msg_destroy(cons, OPCODE_CONSULTA, OBTENER_PEDIDO);

	return receta;
}

void rest_confirmar_pedido(uint32_t pedido_id, int8_t* result) {
	t_consulta* cons = cs_msg_confirmar_ped_rest_create(mi_nombre, pedido_id);
	rest_consultar_sindicato(CONFIRMAR_PEDIDO, cons, result);
	cs_msg_destroy(cons, OPCODE_CONSULTA, CONFIRMAR_PEDIDO);
}

int8_t rest_plato_listo(char* cliente, char* comida, uint32_t pedido_id)
{
	int8_t result;
	t_consulta* plato_listo = cs_msg_plato_listo_create(comida, mi_nombre, pedido_id);
	rest_consultar_sindicato(PLATO_LISTO, plato_listo, &result);
	if(result == OPCODE_RESPUESTA_OK)
	{
		if(cliente == NULL)
		{
			t_sfd conn;
			if(rest_crear_conexion_app(&conn) == STATUS_SUCCESS)
			{
				int8_t result_consulta_app;
				rest_enviar_consulta(MODULO_APP, conn, PLATO_LISTO, plato_listo, &result_consulta_app);
				close(conn);
			}
		} else
		{
			rest_cliente_t* info = rest_cliente_get(cliente);
			if(info != NULL)
			{
				int8_t result_consulta_cliente;
				pthread_mutex_lock(&info->mutex_conexion);
				rest_enviar_consulta(MODULO_CLIENTE, info->conexion, PLATO_LISTO, plato_listo, &result_consulta_cliente);
				pthread_mutex_unlock(&info->mutex_conexion);
			} else
			{
				CS_LOG_WARNING("No se pudo encontrar al cliente para enviar PLATO_LISTO: {ID_CLIENTE: %s}", cliente);
			}
		}
	}
	cs_msg_destroy(plato_listo, OPCODE_CONSULTA, PLATO_LISTO);

	return result == OPCODE_RESPUESTA_OK ? rest_terminar_pedido_si_corresponde(pedido_id) : result;
}

static int8_t rest_terminar_pedido_si_corresponde(uint32_t pedido_id)
{
	int8_t result;
	t_rta_obt_ped* pedido = rest_obtener_pedido(pedido_id, &result);
	if(result == OPCODE_RESPUESTA_OK && pedido->estado_pedido != PEDIDO_TERMINADO)
	{
		switch(cs_platos_estan_listos(pedido->platos_y_estados))
		{
		case 1:
			CS_LOG_TRACE("Se procederá a Terminar: {ID_PEDIDO: %d}", pedido_id);
			t_consulta* cons = cs_msg_term_ped_create(mi_nombre, pedido_id);
			rest_consultar_sindicato(TERMINAR_PEDIDO, cons, &result);
			cs_msg_destroy(cons, OPCODE_CONSULTA, TERMINAR_PEDIDO);
			break;
		case 0:
			CS_LOG_TRACE("No todos los platos están listos para: {ID_PEDIDO: %d}", pedido_id);
			break;
		default:
			CS_LOG_WARNING("La cantidad lista es mayor a la total!! {ID_PEDIDO: %d}", pedido_id);
			break;
		}
		cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
	}

	return result;
}

static void* rest_enviar_consulta(e_module dest, t_sfd conexion, int8_t msg_type, t_consulta* consulta, int8_t* result)
{
	e_status status;
	void* rta;
	char* consulta_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, msg_type);

	//Envía la consulta
	status = cs_send_consulta(conexion, msg_type, consulta, dest);
	if(status == STATUS_SUCCESS)
	{
		CS_LOG_DEBUG("Se envió la consulta a %s: %s", cs_enum_module_to_str(dest), consulta_str);
		rta = rest_recibir_respuesta(dest, conexion, msg_type, result);
	}
	else
	{
		*result = OPCODE_RESPUESTA_FAIL;
		rta = NULL;
		CS_LOG_ERROR("%s -- No se pudo enviar la consulta: %s", cs_enum_status_to_str(status), consulta_str);
	}
	free(consulta_str);

	return rta;
}

static void* rest_recibir_respuesta(e_module src, t_sfd conexion, int8_t msg_type, int8_t* result)
{
	e_status status;
	void* rta;

	//Recibe la respuesta
	void _recibir_respuesta(t_sfd conexion, t_header header_recibido, void* respuesta)
	{
		char* rta_str = cs_msg_to_str(respuesta, header_recibido.opcode,  header_recibido.msgtype);
		CS_LOG_DEBUG("Se recibió la respuesta de %s: %s", cs_enum_module_to_str(src), rta_str);
		free(rta_str);

		//Guarda la respuesta para retornarla
		if(header_recibido.msgtype == msg_type)
		{
			*result = header_recibido.opcode;
			rta = respuesta;
		} else
		{
			*result = OPCODE_RESPUESTA_FAIL;
			rta = NULL;
			printf("eso no me lo esperaba xd");
		}
	}
	status = cs_recv_msg(conexion, _recibir_respuesta);
	if(status != STATUS_SUCCESS)
	{
		*result = OPCODE_RESPUESTA_FAIL;
		rta = NULL;
		CS_LOG_ERROR("%s -- No se pudo recibir la respuesta a %s.", cs_enum_status_to_str(status), cs_enum_msgtype_to_str(msg_type));
	}

	return rta;
}
