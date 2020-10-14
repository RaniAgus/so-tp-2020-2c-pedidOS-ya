#include "apconn.h"

pthread_t hilo_conexiones;
t_sfd	  conexion_escucha;

static void ap_recv_msg_routine(void);

static void ap_recibir_mensaje(t_sfd conn, t_header header, void* msg);

static void ap_recibir_hs_cli(t_sfd conn, t_handshake_cli* msg);
static void ap_recibir_hs_rest(t_sfd conn, t_handshake_res* msg);
static void ap_recibir_cons_rest(t_sfd conn, t_consulta* msg);
static void ap_recibir_sel_rest(t_sfd conn, t_consulta* msg);
static void ap_recibir_cons_pl(t_sfd conn, t_consulta* msg);
static void ap_recibir_crear_ped(t_sfd conn, t_consulta* msg);
static void ap_recibir_aniadir_pl(t_sfd conn, t_consulta* msg);
static void ap_recibir_conf_ped(t_sfd conn, t_consulta* msg);
static void ap_recibir_cons_ped(t_sfd conn, t_consulta* msg);
static void ap_recibir_pl_listo(t_sfd conn, t_consulta* msg);

static void ap_enviar_respuesta(t_sfd conn, e_opcode op_code, e_msgtype msg_type, void* rta);

void ap_conn_init(void)
{
	t_sfd conexion_comanda;
	e_status status;

	// Se intenta conectar con comanda
	status = cs_tcp_client_create(
			&conexion_comanda,
			cs_config_get_string("IP_COMANDA"),
			cs_config_get_string("PUERTO_COMANDA")
	);

	if(status == STATUS_SUCCESS) {
		CS_LOG_TRACE("Conectado correctamente con Comanda {IP: %s} {PUERTO: %s}",
				cs_config_get_string("IP_COMANDA"),
				cs_config_get_string("PUERTO_COMANDA")
		);
		close(conexion_comanda);
	} else {
		PRINT_ERROR(status);
		exit(-1);
	}

	CHECK_STATUS(cs_tcp_server_create(&conexion_escucha, cs_config_get_string("PUERTO_ESCUCHA")));
	CHECK_STATUS(PTHREAD_CREATE(&hilo_conexiones, ap_recv_msg_routine, NULL));
}

// Funciones locales

static void ap_recv_msg_routine(void)
{
	CS_LOG_TRACE("Se abrió un servidor: {PUERTO_ESCUCHA: %s}", cs_config_get_string("PUERTO_ESCUCHA"));

	void _err_handler(e_status err) {
		PRINT_ERROR(err);
	}

	void _recv_msg(t_sfd* conn) {
		e_status status;

		status = cs_recv_msg(*conn, ap_recibir_mensaje);
		if(status != STATUS_SUCCESS) PRINT_ERROR(status);
	}

	cs_tcp_server_accept_routine(&conexion_escucha, _recv_msg, _err_handler);
}

static void ap_recibir_mensaje(t_sfd conn, t_header header, void* msg)
{
	char *msg_str = cs_msg_to_str(msg, header.opcode, header.msgtype);
	CS_LOG_TRACE("Se recibió el mensaje: %s", msg_str);
	free(msg_str);

	if(header.opcode == OPCODE_CONSULTA) //Si es consulta, la atiende
	{
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE:		  ap_recibir_hs_cli    (conn, msg); break;
		case HANDSHAKE_RESTAURANTE:   ap_recibir_hs_rest   (conn, msg); break;
		case CONSULTAR_RESTAURANTES:  ap_recibir_cons_rest (conn, msg); break;
		case SELECCIONAR_RESTAURANTE: ap_recibir_sel_rest  (conn, msg); break;
		case CONSULTAR_PLATOS:		  ap_recibir_cons_pl   (conn, msg); break;
		case CREAR_PEDIDO:			  ap_recibir_crear_ped (conn, msg); break;
		case ANIADIR_PLATO:			  ap_recibir_aniadir_pl(conn, msg); break;
		case CONFIRMAR_PEDIDO:		  ap_recibir_conf_ped  (conn, msg); break;
		case CONSULTAR_PEDIDO:		  ap_recibir_cons_ped  (conn, msg); break;
		case PLATO_LISTO:			  ap_recibir_pl_listo  (conn, msg); break;
		default:
			/* La App NO recibe:
			 * OBTENER RESTAURANTE
			 * GUARDAR/OBTENER/TERMINAR/FINALIZAR PEDIDO
			 * GUARDAR PLATO
			 * OBTENER RECETA */
			//respuesta = NULL;
			//header.opcode = OPCODE_RESPUESTA_FAIL;
			ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, header.msgtype, NULL);
			break;
		}

	} else //Si no es consulta, loguea un error
	{
		CS_LOG_ERROR("Se esperaba una consulta!!");
	}
}

static void ap_recibir_hs_cli(t_sfd conn, t_handshake_cli* msg)
{
	t_header header = { OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE };
	t_rta_handshake_cli* rta;

	//Envía una respuesta handshake
	rta = cs_rta_handshake_cli_create();
	ap_enviar_respuesta(conn, header.opcode, header.msgtype, rta);

	// Si el cliente no se encuentra, se agrega
	if(ap_cliente_find_index(msg->nombre) < 0)
	{
		ap_cliente_add( ap_cliente_create(msg->nombre, msg->posicion, conn) );
	}
	else // Si se encuentra, hay que recibir la consulta
	{
		e_status status = cs_recv_msg(conn, ap_recibir_mensaje);
		if(status != STATUS_SUCCESS)
		{
			PRINT_ERROR(status);
		}
	}

	cs_msg_destroy(msg, OPCODE_CONSULTA, header.msgtype);
	cs_msg_destroy(rta, header.opcode, header.msgtype);
}

static void ap_recibir_hs_rest(t_sfd conn, t_handshake_res* msg)
{


}

static void ap_recibir_cons_rest(t_sfd conn, t_consulta* msg)
{


}

static void ap_recibir_sel_rest(t_sfd conn, t_consulta* msg)
{


}

static void ap_recibir_cons_pl(t_sfd conn, t_consulta* msg)
{


}

static void ap_recibir_crear_ped(t_sfd conn, t_consulta* msg)
{


}

static void ap_recibir_aniadir_pl(t_sfd conn, t_consulta* msg)
{


}

static void ap_recibir_conf_ped(t_sfd conn, t_consulta* msg)
{


}

static void ap_recibir_cons_ped(t_sfd conn, t_consulta* msg)
{


}

static void ap_recibir_pl_listo(t_sfd conn, t_consulta* msg)
{


}


static void ap_enviar_respuesta(t_sfd conn, e_opcode op_code, e_msgtype msg_type, void* rta)
{
	e_status status;
	t_header header = { op_code, msg_type };
	char* respuesta_str = cs_msg_to_str(rta, header.opcode, header.msgtype);

	status = cs_send_respuesta(conn, header, rta);
	if(status == STATUS_SUCCESS)
	{
		CS_LOG_INFO("Se envió la respuesta: %s", respuesta_str);
	}
	else
	{
		CS_LOG_ERROR("%s -- No se pudo enviar la respuesta: %s",
				cs_enum_status_to_str(status),
				respuesta_str
		);
	}

	free(respuesta_str);
}
