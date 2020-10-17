#include "restlisten.h"

static pthread_t hilo_escucha;
static t_sfd	 conexion_escucha;

static void rest_recv_msg_routine(void);

static void rest_recibir_mensaje(t_sfd conn, t_header header, void* msg, char* cliente);
static void rest_recibir_hs_cli(t_sfd conn, t_handshake_cli* msg);
static void rest_recibir_cons_pl(t_sfd conn, t_consulta* msg);
static void rest_recibir_crear_ped(t_sfd conn, t_consulta* msg);
static void rest_recibir_aniadir_pl(t_sfd conn, t_consulta* msg);
static void rest_recibir_conf_ped(t_sfd conn, t_consulta* msg, char* cliente);
static void rest_recibir_cons_ped(t_sfd conn, t_consulta* msg);

static e_status rest_enviar_respuesta(t_sfd conn, e_opcode op_code, e_msgtype msg_type, void* rta);

void rest_listen_init(void)
{
	CHECK_STATUS(cs_tcp_server_create(&conexion_escucha, cs_config_get_string("PUERTO_ESCUCHA")));
	CHECK_STATUS(PTHREAD_CREATE(&hilo_escucha, rest_recv_msg_routine, NULL));
}

static void rest_recv_msg_routine(void)
{
	CS_LOG_TRACE("Se abrió un servidor: {PUERTO_ESCUCHA: %s}", cs_config_get_string("PUERTO_ESCUCHA"));

	void _err_handler(e_status err) {
		PRINT_ERROR(err);
	}

	void _recv_msg(t_sfd* conn) {
		e_status status;

		void _recibir_mensaje(t_sfd conexion, t_header encabezado, void* mensaje) {
			rest_recibir_mensaje(conexion, encabezado, mensaje, NULL);
		}

		status = cs_recv_msg(*conn, _recibir_mensaje);
		if(status != STATUS_SUCCESS) PRINT_ERROR(status);

		free((void*)conn);
	}

	cs_tcp_server_accept_routine(&conexion_escucha, _recv_msg, _err_handler);
}

static void rest_recibir_mensaje(t_sfd conn, t_header header, void* msg, char* cliente)
{
	char *msg_str = cs_msg_to_str(msg, header.opcode, header.msgtype);
	CS_LOG_TRACE("(%d)Se recibió el mensaje: %s", conn, msg_str);
	free(msg_str);

	if(header.opcode == OPCODE_CONSULTA) //Si es consulta, la atiende
	{
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE: rest_recibir_hs_cli    (conn, msg); break;
		case CONSULTAR_PLATOS:	rest_recibir_cons_pl   (conn, msg); break;
		case CREAR_PEDIDO:		rest_recibir_crear_ped (conn, msg); break;
		case ANIADIR_PLATO:		rest_recibir_aniadir_pl(conn, msg); break;
		case CONFIRMAR_PEDIDO:	rest_recibir_conf_ped(conn, msg, cliente); break;
		case CONSULTAR_PEDIDO:	rest_recibir_cons_ped  (conn, msg); break;
		default:
			/* Un Restaurante NO recibe:
			 * HANDSHAKE/OBTENER/CONSULTAR/SELECCIONAR RESTAURANTE
			 * GUARDAR/OBTENER/TERMINAR/FINALIZAR PEDIDO
			 * GUARDAR PLATO/PLATO LISTO
			 * OBTENER RECETA */
			rest_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, header.msgtype, NULL);
			break;
		}

	} else //Si no es consulta, loguea un error
	{
		CS_LOG_ERROR("Se esperaba una consulta!!");
		close(conn);
	}
}

static void rest_recibir_hs_cli(t_sfd conn, t_handshake_cli* msg)
{
	e_status status;
	t_rta_handshake_cli* rta;

	//Crea una respuesta handshake
	rta = cs_rta_handshake_cli_create();

	// Si el cliente no se encuentra, se agrega
	if(rest_cliente_find_index(msg->nombre) < 0)
	{
		CS_LOG_TRACE("(%d)No se encontró a %s entre la lista de Clientes, se agregará a la misma.", conn, msg->nombre);
		rest_cliente_add( rest_cliente_create(msg->nombre, conn) );

		//Después de agregar, se envía la RESPUESTA_OK
		rest_enviar_respuesta(conn, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE, rta);
	}
	else // Si se encuentra, primero hay que enviar RESPUESTA_OK para recibir la consulta
	{
		CS_LOG_TRACE("(%d)Se encontró a %s entre la lista de Clientes, se recibirá la consulta.", conn, msg->nombre);
		if( rest_enviar_respuesta(conn, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE, rta) == STATUS_SUCCESS )
		{
			void _recibir_mensaje(t_sfd conn, t_header header, void* mensaje) {
				rest_recibir_mensaje(conn, header, mensaje, msg->nombre);
			}
			status = cs_recv_msg(conn, _recibir_mensaje);
			if(status != STATUS_SUCCESS) {
				PRINT_ERROR(status);
			} else {
				CS_LOG_TRACE("(%d)Se atendió la consulta del Cliente %s", conn, msg->nombre);
			}
		}
	}

	cs_msg_destroy(msg, OPCODE_CONSULTA, HANDSHAKE_CLIENTE);
	cs_msg_destroy(rta, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE);
}

static void rest_recibir_cons_pl(t_sfd conn, t_consulta* msg)
{
	int8_t resultado_cons;
	t_consulta* cons;
	t_rta_cons_pl* rta;

	//Reenvía la consulta al Sindicato
	cons = cs_msg_consultar_pl_rest_create(mi_nombre);
	rta = rest_consultar_sindicato(CONSULTAR_PLATOS, cons, &resultado_cons);

	//Retorna la respuesta obtenida
	rest_enviar_respuesta(conn, resultado_cons, CONSULTAR_PLATOS, rta);

	cs_msg_destroy(rta , resultado_cons , CONSULTAR_PLATOS);
	cs_msg_destroy(cons, OPCODE_CONSULTA, CONSULTAR_PLATOS);
	cs_msg_destroy(msg , OPCODE_CONSULTA, CONSULTAR_PLATOS);
}

static void rest_recibir_crear_ped(t_sfd conn, t_consulta* msg)
{
	int8_t resultado_cons;
	t_consulta* cons;
	t_rta_crear_ped* rta = NULL;

	//Envía GUARDAR_PEDIDO generando un ID único para el Restaurante
	cons = cs_msg_guardar_ped_create(mi_nombre, rest_generar_id());
	rest_consultar_sindicato(GUARDAR_PEDIDO, cons, &resultado_cons);

	//Si obtuvo una RESPUESTA_OK, genera la respuesta a CREAR_PEDIDO
	if(resultado_cons == OPCODE_RESPUESTA_OK) {
		rta = cs_rta_crear_ped_create(cons->pedido_id);
	}

	//Retorna la respuesta generada
	rest_enviar_respuesta(conn, resultado_cons, CREAR_PEDIDO, rta);

	cs_msg_destroy(rta , resultado_cons , CREAR_PEDIDO);
	cs_msg_destroy(cons, OPCODE_CONSULTA, GUARDAR_PEDIDO);
	cs_msg_destroy(msg , OPCODE_CONSULTA, CREAR_PEDIDO);
}

static void rest_recibir_aniadir_pl(t_sfd conn, t_consulta* msg)
{
	int8_t resultado_cons;
	t_consulta* cons;

	//Envía GUARDAR_PLATO generando un ID único para el Restaurante
	cons = cs_msg_guardar_pl_create(msg->comida, 1, mi_nombre, msg->pedido_id);
	rest_consultar_sindicato(GUARDAR_PLATO, cons, &resultado_cons);

	//Retorna la respuesta generada
	rest_enviar_respuesta(conn, resultado_cons, ANIADIR_PLATO, NULL);

	cs_msg_destroy(cons, OPCODE_CONSULTA, GUARDAR_PLATO);
	cs_msg_destroy(msg , OPCODE_CONSULTA, ANIADIR_PLATO);
}

static void rest_recibir_conf_ped(t_sfd conn, t_consulta* msg, char* cliente)
{
	int8_t resultado_cons;
	t_rta_obt_ped* pedido;

	//Obtiene el pedido
	pedido = rest_obtener_pedido(msg->pedido_id, &resultado_cons);

	//Si se obtuvo con éxito, genera los PCBs de cada plato
	if(resultado_cons == OPCODE_RESPUESTA_OK) {
		rest_generar_pcbs(msg->pedido_id, pedido, cliente);
	}

	//Retorna la respuesta obtenida
	rest_enviar_respuesta(conn, resultado_cons, CONFIRMAR_PEDIDO, NULL);

	cs_msg_destroy(pedido, resultado_cons, OBTENER_PEDIDO);
	cs_msg_destroy(msg, OPCODE_CONSULTA, ANIADIR_PLATO);
}

static void rest_recibir_cons_ped(t_sfd conn, t_consulta* msg)
{
	int8_t resultado_cons;
	t_rta_obt_ped* pedido;
	t_rta_cons_ped* rta = NULL;

	//Envía OBTENER_PEDIDO
	pedido = rest_obtener_pedido(msg->pedido_id, &resultado_cons);

	//Si obtuvo una RESPUESTA_OK, genera la respuesta a CONSULTAR_PEDIDO
	if(resultado_cons == OPCODE_RESPUESTA_OK) {
		rta = malloc(sizeof(t_rta_cons_ped));
		rta->restaurante = mi_nombre;
		rta->estado_pedido = pedido->estado_pedido;
		rta->platos_y_estados = pedido->platos_y_estados;
	}

	//Retorna la respuesta generada
	rest_enviar_respuesta(conn, resultado_cons, CONSULTAR_PEDIDO, rta);

	cs_msg_destroy(pedido, resultado_cons, OBTENER_PEDIDO);
	if(resultado_cons == OPCODE_RESPUESTA_OK) free(rta);
	cs_msg_destroy(msg, OPCODE_CONSULTA, CONSULTAR_PEDIDO);
}

static e_status rest_enviar_respuesta(t_sfd conn, e_opcode op_code, e_msgtype msg_type, void* rta)
{
	e_status status;
	t_header header = { op_code, msg_type };
	char* respuesta_str = cs_msg_to_str(rta, header.opcode, header.msgtype);

	status = cs_send_respuesta(conn, header, rta);
	if(status == STATUS_SUCCESS)
	{
		CS_LOG_TRACE("Se envió la respuesta: %s", respuesta_str);
	}
	else
	{
		CS_LOG_ERROR("%s -- No se pudo enviar la respuesta: %s",
				cs_enum_status_to_str(status),
				respuesta_str
		);
	}

	free(respuesta_str);
	return status;
}

