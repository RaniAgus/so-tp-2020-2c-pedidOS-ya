#include "restrecepcion.h"

static pthread_t hilo_escucha;
static t_sfd	 conexion_escucha;

static void rest_recv_msg_routine(void);

static void rest_recibir_mensaje(t_sfd conn, t_header header, void* msg, char* cliente);
static void rest_recibir_handshake_cliente(t_sfd conn, t_handshake_cli* recibido);
static void rest_recibir_consultar_platos(t_sfd conn, t_consulta* recibido);
static void rest_recibir_crear_pedido(t_sfd conn, t_consulta* recibido);
static void rest_recibir_aniadir_plato(t_sfd conn, t_consulta* msg);
static void rest_recibir_confirmar_pedido(t_sfd conn, t_consulta* recibido, char* cliente);
static void rest_recibir_consultar_pedido(t_sfd conn, t_consulta* recibido);

static e_status rest_enviar_respuesta(t_sfd conn, e_opcode op_code, e_msgtype msg_type, void* rta);

void rest_recepcion_init(void)
{
	char* puerto_escucha = cs_config_get_string("PUERTO_ESCUCHA");
	CHECK_STATUS(cs_tcp_server_create(&conexion_escucha, puerto_escucha));
	CS_LOG_TRACE("Se abrió un servidor: {PUERTO_ESCUCHA: %s}", puerto_escucha);
	pthread_create(&hilo_escucha, NULL, (void*)rest_recv_msg_routine, NULL);
	rest_clientes_init();
}

static void rest_recv_msg_routine(void)
{
	void _recv_msg(t_sfd* conexion_cliente)
	{
		e_status status;

		void _recibir_mensaje(t_sfd conexion, t_header encabezado, void* mensaje) {
			rest_recibir_mensaje(conexion, encabezado, mensaje, NULL);
		}
		status = cs_recv_msg(*conexion_cliente, _recibir_mensaje);
		if(status != STATUS_SUCCESS) PRINT_ERROR(status);

		free((void*)conexion_cliente);
	}

	void _err_handler(e_status err) { PRINT_ERROR(err); }

	cs_tcp_server_accept_routine(&conexion_escucha, _recv_msg, _err_handler);
}

static void rest_recibir_mensaje(t_sfd conn, t_header header, void* msg, char* cliente)
{
	char *msg_str = cs_msg_to_str(msg, header.opcode, header.msgtype);
	CS_LOG_DEBUG("(%d)Se recibió el mensaje: %s", conn, msg_str);
	free(msg_str);

	if(header.opcode == OPCODE_CONSULTA) //Si es consulta, la atiende
	{
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE: rest_recibir_handshake_cliente(conn, msg); break;
		case CONSULTAR_PLATOS:	rest_recibir_consultar_platos (conn, msg); break;
		case CREAR_PEDIDO:		rest_recibir_crear_pedido     (conn, msg); break;
		case ANIADIR_PLATO:		rest_recibir_aniadir_plato    (conn, msg); break;
		case CONFIRMAR_PEDIDO:	rest_recibir_confirmar_pedido (conn, msg, cliente); break;
		case CONSULTAR_PEDIDO:	rest_recibir_consultar_pedido (conn, msg); break;
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

static void rest_recibir_handshake_cliente(t_sfd conn, t_handshake_cli* recibido)
{
	e_status status;
	t_rta_handshake_cli* respuesta;

	//Crea una respuesta handshake
	respuesta = cs_rta_handshake_cli_create();

	// Si el cliente no se encuentra, se agrega
	if(!rest_cliente_is_connected(recibido->nombre))
	{
		CS_LOG_TRACE("(%d)No se encontró a %s entre la lista de Clientes, se agregará a la misma.", conn, recibido->nombre);
		rest_cliente_connect(recibido->nombre, conn);

		//Después de agregar, se envía la RESPUESTA_OK
		rest_enviar_respuesta(conn, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE, respuesta);
	}
	else // Si se encuentra, primero hay que enviar RESPUESTA_OK para recibir la consulta
	{
		CS_LOG_TRACE("(%d)Se encontró a %s entre la lista de Clientes, se recibirá la consulta.", conn, recibido->nombre);
		if( rest_enviar_respuesta(conn, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE, respuesta) == STATUS_SUCCESS )
		{
			void _recibir_mensaje(t_sfd conn, t_header header, void* mensaje) {
				rest_recibir_mensaje(conn, header, mensaje, recibido->nombre);
			}
			status = cs_recv_msg(conn, _recibir_mensaje);
			if(status != STATUS_SUCCESS) {
				PRINT_ERROR(status);
			} else {
				CS_LOG_TRACE("(%d)Se atendió la consulta del Cliente %s", conn, recibido->nombre);
			}
		}
	}

	cs_msg_destroy(recibido, OPCODE_CONSULTA, HANDSHAKE_CLIENTE);
	cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE);
}

static void rest_recibir_consultar_platos(t_sfd conn, t_consulta* recibido)
{
	int8_t resultado;
	t_consulta* consulta;
	t_rta_cons_pl* respuesta;

	//Reenvía la consulta al Sindicato
	consulta = cs_msg_consultar_pl_rest_create(mi_nombre);
	respuesta = rest_consultar_sindicato(CONSULTAR_PLATOS, consulta, &resultado);

	//Retorna la respuesta obtenida
	rest_enviar_respuesta(conn, resultado, CONSULTAR_PLATOS, respuesta);

	cs_msg_destroy(respuesta, resultado      , CONSULTAR_PLATOS);
	cs_msg_destroy(consulta , OPCODE_CONSULTA, CONSULTAR_PLATOS);
	cs_msg_destroy(recibido , OPCODE_CONSULTA, CONSULTAR_PLATOS);
}

static void rest_recibir_crear_pedido(t_sfd conn, t_consulta* recibido)
{
	int8_t resultado;
	t_consulta* consulta;
	t_rta_crear_ped* respuesta = NULL;

	//Envía GUARDAR_PEDIDO generando un ID único para el Restaurante
	consulta = cs_msg_guardar_ped_create(mi_nombre, rest_generar_id());
	rest_consultar_sindicato(GUARDAR_PEDIDO, consulta, &resultado);

	//Si obtuvo una RESPUESTA_OK, genera la respuesta a CREAR_PEDIDO
	if(resultado == OPCODE_RESPUESTA_OK) {
		respuesta = cs_rta_crear_ped_create(consulta->pedido_id);
	}

	//Retorna la respuesta generada
	rest_enviar_respuesta(conn, resultado, CREAR_PEDIDO, respuesta);

	cs_msg_destroy(respuesta, resultado      , CREAR_PEDIDO  );
	cs_msg_destroy(consulta , OPCODE_CONSULTA, GUARDAR_PEDIDO);
	cs_msg_destroy(recibido , OPCODE_CONSULTA, CREAR_PEDIDO  );
}

static void rest_recibir_aniadir_plato(t_sfd conn, t_consulta* msg)
{
	int8_t resultado;
	t_consulta* consulta;

	//Envía GUARDAR_PLATO
	consulta = cs_msg_guardar_pl_create(msg->comida, 1, mi_nombre, msg->pedido_id);
	rest_consultar_sindicato(GUARDAR_PLATO, consulta, &resultado);

	//Retorna la respuesta obtenida
	rest_enviar_respuesta(conn, resultado, ANIADIR_PLATO, NULL);

	cs_msg_destroy(consulta, OPCODE_CONSULTA, GUARDAR_PLATO);
	cs_msg_destroy(msg , OPCODE_CONSULTA, ANIADIR_PLATO);
}

static void rest_recibir_confirmar_pedido(t_sfd conn, t_consulta* recibido, char* cliente)
{
	int8_t resultado;
	t_rta_obt_ped* pedido;

	//Obtiene el pedido
	pedido = rest_obtener_pedido(recibido->pedido_id, &resultado);

	//Si se obtuvo con éxito, genera los PCBs de cada plato
	if(resultado == OPCODE_RESPUESTA_OK)
	{
		CS_LOG_TRACE("Se generarán los PCBs del pedido %d generado por %s",
				recibido->pedido_id, cliente ? cliente : "App"
		);

		//Itera la lista de platos
		void _generar_plato_control_block(t_plato* plato)
		{
			int8_t resultado_obtener_receta;

			//Obtiene la receta de la comida
			t_rta_obt_rec* receta = rest_obtener_receta(plato->comida, &resultado_obtener_receta);
			if(resultado_obtener_receta == OPCODE_RESPUESTA_OK)
			{
				//Crea un PCB para cada plato
				for(int i = 0; i < plato->cant_total; i++) {
					if(rest_planificar_plato(plato->comida, recibido->pedido_id, receta->pasos_receta, cliente) == -1) {
						CS_LOG_ERROR("No se pudo planificar el plato: %s", plato->comida);
						resultado = OPCODE_RESPUESTA_FAIL;
						break;
					}
				}
			}
			else
			{
				CS_LOG_ERROR("Error al obtener la receta del plato: %s", plato->comida);
				resultado = OPCODE_RESPUESTA_FAIL;
			}

			cs_msg_destroy(receta, resultado_obtener_receta, OBTENER_RECETA);
		}
		list_iterate(pedido->platos_y_estados, (void*) _generar_plato_control_block);
	}

	//Retorna la respuesta obtenida
	rest_enviar_respuesta(conn, resultado, CONFIRMAR_PEDIDO, NULL);

	cs_msg_destroy(pedido  , resultado      , OBTENER_PEDIDO);
	cs_msg_destroy(recibido, OPCODE_CONSULTA, ANIADIR_PLATO );
}

static void rest_recibir_consultar_pedido(t_sfd conn, t_consulta* recibido)
{
	int8_t resultado;
	t_rta_obt_ped* pedido;
	t_rta_cons_ped* respuesta = NULL;

	//Envía OBTENER_PEDIDO
	pedido = rest_obtener_pedido(recibido->pedido_id, &resultado);

	//Si obtuvo una RESPUESTA_OK, genera la respuesta a CONSULTAR_PEDIDO
	if(resultado == OPCODE_RESPUESTA_OK) {
		respuesta = malloc(sizeof(t_rta_cons_ped));
		respuesta->restaurante = mi_nombre;
		respuesta->estado_pedido = pedido->estado_pedido;
		respuesta->platos_y_estados = pedido->platos_y_estados;
	}

	//Retorna la respuesta generada
	rest_enviar_respuesta(conn, resultado, CONSULTAR_PEDIDO, respuesta);

	cs_msg_destroy(pedido, resultado, OBTENER_PEDIDO);
	if(resultado == OPCODE_RESPUESTA_OK) free(respuesta);
	cs_msg_destroy(recibido, OPCODE_CONSULTA, CONSULTAR_PEDIDO);
}

static e_status rest_enviar_respuesta(t_sfd conn, e_opcode op_code, e_msgtype msg_type, void* rta)
{
	e_status status;
	t_header header = { op_code, msg_type };
	char* respuesta_str = cs_msg_to_str(rta, header.opcode, header.msgtype);

	status = cs_send_respuesta(conn, header, rta);
	if(status == STATUS_SUCCESS) {
		CS_LOG_DEBUG("Se envió la respuesta: %s", respuesta_str);
	} else {
		CS_LOG_ERROR("%s -- No se pudo enviar la respuesta: %s", cs_enum_status_to_str(status), respuesta_str);
	}

	free(respuesta_str);
	return status;
}

