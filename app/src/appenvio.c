#include "appenvio.h"

static void* app_consultar_comanda(int8_t msg_type, t_consulta* consulta, int8_t* result_ptr);
static void* app_enviar_consulta(e_module dest, t_sfd conexion, int8_t msg_type, t_consulta* consulta, int8_t* result_ptr);

void app_plato_listo(t_consulta* consulta, int8_t* result_ptr)
{
	app_consultar_comanda(PLATO_LISTO, consulta, result_ptr);
}

int8_t app_finalizar_pedido(char* restaurante, uint32_t pedido_id, char* cliente)
{
	int8_t result;
	t_consulta* finalizar_pedido = cs_msg_fin_ped_create(restaurante, pedido_id);

	app_consultar_comanda(FINALIZAR_PEDIDO, finalizar_pedido, &result);
	if(result == OPCODE_RESPUESTA_OK)
	{
		app_cliente_t* info_cliente = app_obtener_cliente(cliente);
		pthread_mutex_lock(&info_cliente->mutex_conexion);
		app_enviar_consulta(MODULO_CLIENTE, info_cliente->conexion, FINALIZAR_PEDIDO, finalizar_pedido, &result);
		pthread_mutex_unlock(&info_cliente->mutex_conexion);
	}

	cs_msg_destroy(finalizar_pedido, OPCODE_CONSULTA, FINALIZAR_PEDIDO);
	return result;
}

//Consultar Pedido --> OBTENER_PEDIDO
t_rta_obt_ped* app_obtener_pedido(char* restaurante, uint32_t pedido_id, int8_t* result_ptr)
{
	t_consulta* cons_obtener_pedido = cs_msg_obtener_ped_create(restaurante, pedido_id);
	t_rta_obt_ped* rta_obtener_pedido = app_consultar_comanda(OBTENER_PEDIDO, cons_obtener_pedido, result_ptr);
	cs_msg_destroy(cons_obtener_pedido, OPCODE_CONSULTA, OBTENER_PEDIDO);

	return rta_obtener_pedido;
}

//Crear Pedido ---> GUARDAR_PEDIDO
void app_guardar_pedido(char* restaurante, uint32_t pedido_id, int8_t* result_ptr)
{
	t_consulta* guardar_ped = cs_msg_guardar_ped_create(restaurante, pedido_id);
	app_consultar_comanda(GUARDAR_PEDIDO, guardar_ped, result_ptr);
	cs_msg_destroy(guardar_ped, OPCODE_CONSULTA, GUARDAR_PEDIDO);
}

//Añadir Plato --> GUARDAR_PLATO
void app_guardar_plato(char* comida, char* restaurante, uint32_t pedido_id, int8_t* result_ptr)
{
	t_consulta* consulta = cs_msg_guardar_pl_create(comida, 1, restaurante, pedido_id);
	app_consultar_comanda(GUARDAR_PLATO, consulta, result_ptr);
	cs_msg_destroy(consulta, OPCODE_CONSULTA, GUARDAR_PLATO);
}

void app_confirmar_pedido(char* restaurante, uint32_t pedido_id, int8_t* result)
{
	t_consulta* consulta = cs_msg_confirmar_ped_rest_create(restaurante, pedido_id);
	app_consultar_comanda(CONFIRMAR_PEDIDO, consulta, result);
	cs_msg_destroy(consulta, OPCODE_CONSULTA, CONFIRMAR_PEDIDO);
}

//"Pasamano" de: CONSULTAR_PLATOS, CREAR_PEDIDO, ANIADIR_PLATO, CONFIRMAR_PEDIDO
void* app_consultar_restaurante(char* restaurante, int8_t msg_type, t_consulta* consulta, int8_t* result_ptr)
{
	void* respuesta = NULL;

	char *ip, *puerto;
	if(app_address_restaurante(restaurante, &ip, &puerto))
	{
		e_status status;
		t_sfd conexion_restaurante;

		//Se conecta como cliente
		status = cs_tcp_client_create(&conexion_restaurante, ip, puerto);
		if(status != STATUS_SUCCESS) {
			CS_LOG_ERROR("%s -- No se pudo conectar con Restaurante %s:%s", cs_enum_status_to_str(status), ip, puerto);
		} else {
			CS_LOG_TRACE("Conectado exitosamente con Restaurante %s:%s", ip, puerto);
		}

		respuesta =  app_enviar_consulta(MODULO_RESTAURANTE, conexion_restaurante, msg_type, consulta, result_ptr);

		close(conexion_restaurante);
		free(ip);
		free(puerto);
	} else
	{
		*result_ptr = OPCODE_RESPUESTA_FAIL;
		CS_LOG_ERROR("No se encontró como conectado: {RESTAURANTE: %s}", restaurante);
	}

	return respuesta;
}

// Funciones locales

static void* app_consultar_comanda(int8_t msg_type, t_consulta* consulta, int8_t* result_ptr)
{
	e_status status;
	t_sfd conexion_comanda;
	void* respuesta;

	//Se conecta como cliente
	status = cs_tcp_client_create(&conexion_comanda,
			cs_config_get_string("IP_COMANDA"),
			cs_config_get_string("PUERTO_COMANDA")
	);
	if(status != STATUS_SUCCESS)
	{
		CS_LOG_ERROR("%s -- No se pudo conectar con Comanda. Finalizando.", cs_enum_status_to_str(status));
		exit(-1);
	}
	CS_LOG_TRACE("Conectado exitosamente con Comanda.");

	respuesta = app_enviar_consulta(MODULO_COMANDA, conexion_comanda, msg_type, consulta, result_ptr);
	close(conexion_comanda);

	return respuesta;
}

static void* app_enviar_consulta(e_module dest, t_sfd conexion, int8_t msg_type, t_consulta* consulta, int8_t* result_ptr)
{
	e_status status;
	void* respuesta;
	char* consulta_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, msg_type);

	//Envía la consulta
	status = cs_send_consulta(conexion, msg_type, consulta, dest);
	if(status == STATUS_SUCCESS)
	{
		CS_LOG_TRACE("Se envió la consulta: %s", consulta_str);

		//Recibe la respuesta
		void _recibir_respuesta(t_sfd conexion, t_header header_recibido, void* rta_recibida)
		{
			char* rta_str = cs_msg_to_str(rta_recibida, header_recibido.opcode,  header_recibido.msgtype);
			CS_LOG_TRACE("Se recibió la respuesta: %s", rta_str);
			free(rta_str);

			//Guarda la respuesta para retornarla
			if( header_recibido.msgtype == msg_type ) {
				*result_ptr = header_recibido.opcode;
				respuesta = rta_recibida;
			} else {
				*result_ptr = OPCODE_RESPUESTA_FAIL;
				respuesta = NULL;
				CS_LOG_ERROR("a ver a ver, qué pasó?");
			}
		}
		status = cs_recv_msg(conexion, _recibir_respuesta);
		if(status != STATUS_SUCCESS)
		{
			*result_ptr = OPCODE_RESPUESTA_FAIL;
			respuesta = NULL;
			CS_LOG_ERROR("%s -- No se pudo recibir la respuesta a: %s", cs_enum_status_to_str(status), consulta_str);
		}
	}
	else
	{
		*result_ptr = OPCODE_RESPUESTA_FAIL;
		respuesta = NULL;
		CS_LOG_ERROR("%s -- No se pudo enviar la consulta: %s", cs_enum_status_to_str(status), consulta_str);
	}
	free(consulta_str);

	return respuesta;
}
