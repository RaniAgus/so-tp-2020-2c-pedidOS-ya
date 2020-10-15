#include "apsend.h"

static void* ap_consultar_comanda(int8_t msg_type, t_consulta* consulta, int8_t* result);
static void* ap_enviar_consulta(e_module dest, t_sfd conexion, int8_t msg_type, t_consulta* consulta, int8_t* result);

int8_t ap_finalizar_pedido(char* restaurante, uint32_t pedido_id, char* cliente)
{
	int8_t result;

	t_consulta* finalizar_pedido = cs_msg_fin_ped_create(restaurante, pedido_id);

	ap_consultar_comanda(FINALIZAR_PEDIDO, finalizar_pedido, &result);
	if(result == OPCODE_RESPUESTA_OK)
	{
		void _enviar_mensaje_al_cliente(ap_cliente_t* encontrado) {
			pthread_mutex_lock(&encontrado->mutex_conexion);
			ap_enviar_consulta(
					MODULO_CLIENTE,
					encontrado->conexion,
					FINALIZAR_PEDIDO,
					finalizar_pedido,
					&result
			);
			pthread_mutex_unlock(&encontrado->mutex_conexion);
		}
		ap_cliente_find(cliente, _enviar_mensaje_al_cliente);
	}

	cs_msg_destroy(finalizar_pedido, OPCODE_CONSULTA, FINALIZAR_PEDIDO);
	return result;
}


//Consultar Pedido --> OBTENER_PEDIDO
t_rta_obt_ped* ap_obtener_pedido(char* restaurante, uint32_t pedido_id, int8_t* result)
{
	t_consulta* cons_obtener_pedido = cs_msg_obtener_ped_create(restaurante, pedido_id);
	t_rta_obt_ped* rta_obtener_pedido = ap_consultar_comanda(OBTENER_PEDIDO, cons_obtener_pedido, result);
	cs_msg_destroy(cons_obtener_pedido, OPCODE_CONSULTA, OBTENER_PEDIDO);

	return rta_obtener_pedido;
}

//Crear Pedido ---> GUARDAR_PEDIDO
void ap_guardar_pedido(char* restaurante, uint32_t pedido_id, int8_t* result)
{
	t_consulta* guardar_ped = cs_msg_guardar_ped_create(restaurante, pedido_id);
	ap_consultar_comanda(GUARDAR_PEDIDO, guardar_ped, result);
	cs_msg_destroy(guardar_ped, OPCODE_CONSULTA, GUARDAR_PEDIDO);
}

//Añadir Plato --> GUARDAR_PLATO
void ap_guardar_plato(char* comida, char* restaurante, uint32_t pedido_id, int8_t* result)
{
	t_consulta* consulta = cs_msg_guardar_pl_create(comida, 1, restaurante, pedido_id);
	ap_consultar_comanda(GUARDAR_PLATO, consulta, result);
	cs_msg_destroy(consulta, OPCODE_CONSULTA, GUARDAR_PLATO);
}

void ap_confirmar_pedido(char* restaurante, uint32_t pedido_id, int8_t* result)
{
	t_consulta* consulta = cs_msg_confirmar_ped_rest_create(restaurante, pedido_id);
	ap_consultar_comanda(CONFIRMAR_PEDIDO, consulta, result);
	cs_msg_destroy(consulta, OPCODE_CONSULTA, CONFIRMAR_PEDIDO);
}

//"Pasamano" de: CONSULTAR_PLATOS, CREAR_PEDIDO, ANIADIR_PLATO, CONFIRMAR_PEDIDO
void* ap_consultar_restaurante(char* ip, char* puerto, int8_t msg_type, t_consulta* consulta, int8_t* result)
{
	e_status status;
	t_sfd conexion_restaurante;

	//Se conecta como cliente
	status = cs_tcp_client_create(&conexion_restaurante, ip, puerto);
	if(status != STATUS_SUCCESS)
	{
		CS_LOG_ERROR("%s -- No se pudo conectar con Restaurante %s:%s",
				cs_enum_status_to_str(status), ip, puerto
		);
	}
	else
	{
		CS_LOG_TRACE("Conectado exitosamente con Restaurante %s:%s", ip, puerto);
	}

	return ap_enviar_consulta(MODULO_RESTAURANTE, conexion_restaurante, msg_type, consulta, result);
}

// Funciones locales

static void* ap_consultar_comanda(int8_t msg_type, t_consulta* consulta, int8_t* result)
{
	e_status status;
	t_sfd conexion_comanda;

	//Se conecta como cliente
	status = cs_tcp_client_create(&conexion_comanda,
			cs_config_get_string("IP_COMANDA"),
			cs_config_get_string("PUERTO_COMANDA")
	);
	if(status != STATUS_SUCCESS)
	{
		CS_LOG_ERROR("%s -- No se pudo conectar con Comanda. Finalizando.",
				cs_enum_status_to_str(status)
		);
		exit(-1);
	}
	CS_LOG_TRACE("Conectado exitosamente con Comanda.");

	return ap_enviar_consulta(MODULO_COMANDA, conexion_comanda, msg_type, consulta, result);
}

static void* ap_enviar_consulta(e_module dest, t_sfd conexion_comanda, int8_t msg_type, t_consulta* consulta, int8_t* result)
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
				CS_LOG_ERROR("A ver a ver, ¿qué pasó?");
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
