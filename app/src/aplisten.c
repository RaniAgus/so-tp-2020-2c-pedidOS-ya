#include "aplisten.h"

static pthread_t hilo_escucha;
static t_sfd	 conexion_escucha;

static uint32_t  	   id_default;
static pthread_mutex_t mutex_id_default;

static void ap_recv_msg_routine(void);

static void ap_recibir_mensaje(t_sfd conn, t_header header, void* msg, char* cliente);

static void ap_recibir_hs_cli(t_sfd conn, t_handshake_cli* msg);
static void ap_recibir_hs_rest(t_sfd conn, t_handshake_res* msg);
static void ap_recibir_cons_rest(t_sfd conn, t_consulta* msg);
static void ap_recibir_sel_rest(t_sfd conn, t_consulta* msg, char* cliente);
static void ap_recibir_cons_pl(t_sfd conn, t_consulta* msg, char* cliente);
static void ap_recibir_crear_ped(t_sfd conn, t_consulta* msg, char* cliente);
static void ap_recibir_aniadir_pl(t_sfd conn, t_consulta* msg, char* cliente);
static void ap_recibir_conf_ped(t_sfd conn, t_consulta* msg, char* cliente);
static void ap_recibir_cons_ped(t_sfd conn, t_consulta* msg, char* cliente);
static void ap_recibir_pl_listo(t_sfd conn, t_consulta* msg);

static e_status ap_enviar_respuesta(t_sfd conn, e_opcode op_code, e_msgtype msg_type, void* rta);

void ap_listen_init(void)
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

	id_default = 0;
	pthread_mutex_init(&mutex_id_default, NULL);

	CHECK_STATUS(cs_tcp_server_create(&conexion_escucha, cs_config_get_string("PUERTO_ESCUCHA")));
	CHECK_STATUS(PTHREAD_CREATE(&hilo_escucha, ap_recv_msg_routine, NULL));
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

		void _recibir_mensaje(t_sfd conexion, t_header encabezado, void* mensaje) {
			ap_recibir_mensaje(conexion, encabezado, mensaje, NULL);
		}

		status = cs_recv_msg(*conn, _recibir_mensaje);
		if(status != STATUS_SUCCESS) PRINT_ERROR(status);

		free((void*)conn);
	}

	cs_tcp_server_accept_routine(&conexion_escucha, _recv_msg, _err_handler);
}

static void ap_recibir_mensaje(t_sfd conn, t_header header, void* msg, char* cliente)
{
	char *msg_str = cs_msg_to_str(msg, header.opcode, header.msgtype);
	CS_LOG_TRACE("(%d)Se recibió el mensaje: %s", conn, msg_str);
	free(msg_str);

	if(header.opcode == OPCODE_CONSULTA) //Si es consulta, la atiende
	{
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE:		  ap_recibir_hs_cli    (conn, msg); break;
		case HANDSHAKE_RESTAURANTE:   ap_recibir_hs_rest   (conn, msg); break;
		case CONSULTAR_RESTAURANTES:  ap_recibir_cons_rest (conn, msg); break;
		case SELECCIONAR_RESTAURANTE: ap_recibir_sel_rest  (conn, msg, cliente); break;
		case CONSULTAR_PLATOS:		  ap_recibir_cons_pl   (conn, msg, cliente); break;
		case CREAR_PEDIDO:			  ap_recibir_crear_ped (conn, msg, cliente); break;
		case ANIADIR_PLATO:			  ap_recibir_aniadir_pl(conn, msg, cliente); break;
		case CONFIRMAR_PEDIDO:		  ap_recibir_conf_ped  (conn, msg, cliente); break;
		case CONSULTAR_PEDIDO:		  ap_recibir_cons_ped  (conn, msg, cliente); break;
		case PLATO_LISTO:			  ap_recibir_pl_listo  (conn, msg); break;
		default:
			/* La App NO recibe:
			 * OBTENER RESTAURANTE
			 * GUARDAR/OBTENER/TERMINAR/FINALIZAR PEDIDO
			 * GUARDAR PLATO
			 * OBTENER RECETA */
			ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, header.msgtype, NULL);
			break;
		}

	} else //Si no es consulta, loguea un error
	{
		CS_LOG_ERROR("Se esperaba una consulta!!");
		close(conn);
	}
}

static void ap_recibir_hs_cli(t_sfd conn, t_handshake_cli* msg)
{
	e_status status;
	t_rta_handshake_cli* rta;

	//Crea una respuesta handshake
	rta = cs_rta_handshake_cli_create();

	// Si el cliente no se encuentra, se agrega
	if(ap_cliente_find_index(msg->nombre) < 0)
	{
		CS_LOG_TRACE("(%d)No se encontró a %s entre la lista de Clientes, se agregará a la misma.", conn, msg->nombre);
		ap_cliente_add( ap_cliente_create(msg->nombre, msg->posicion, conn) );

		//Después de agregar, se envía la RESPUESTA_OK
		ap_enviar_respuesta(conn, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE, rta);
	}
	else // Si se encuentra, primero hay que enviar RESPUESTA_OK para recibir la consulta
	{
		CS_LOG_TRACE("(%d)Se encontró a %s entre la lista de Clientes, se recibirá la consulta.", conn, msg->nombre);
		if( ap_enviar_respuesta(conn, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE, rta) == STATUS_SUCCESS )
		{
			void _recibir_mensaje(t_sfd conn, t_header header, void* mensaje) {
				ap_recibir_mensaje(conn, header, mensaje, msg->nombre);
			}
			status = cs_recv_msg(conn, _recibir_mensaje);
			if(status != STATUS_SUCCESS) {
				PRINT_ERROR(status);
			}
		}
	}

	cs_msg_destroy(msg, OPCODE_CONSULTA, HANDSHAKE_CLIENTE);
	cs_msg_destroy(rta, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE);
}

static void ap_recibir_hs_rest(t_sfd conn, t_handshake_res* msg)
{
	e_status status;

	//Se agrega a la lista de restaurantes y se envía RESPUESTA_OK
	ap_restaurante_add(ap_restaurante_create(msg->nombre, msg->posicion, msg->ip, msg->puerto));
	status = ap_enviar_respuesta(conn, OPCODE_RESPUESTA_OK, HANDSHAKE_RESTAURANTE, NULL);
	cs_msg_destroy(msg, OPCODE_CONSULTA, HANDSHAKE_RESTAURANTE);

	//Si se envió correctamente, recibe mensajes del Restaurante por este socket
	CS_LOG_TRACE("(%d)Inició la conexión con el Restaurante.", conn);
	while(status == STATUS_SUCCESS)
	{
		void _recibir_mensaje(t_sfd conexion, t_header encabezado, void* mensaje) {
			ap_recibir_mensaje(conexion, encabezado, mensaje, NULL);
		}
		status = cs_recv_msg(conn, _recibir_mensaje);
		if(status != STATUS_SUCCESS) {
			PRINT_ERROR(status);
		}
	}
	CS_LOG_TRACE("(%d)Se cerró la conexión con el Restaurante.", conn);

	close(conn);
}

static void ap_recibir_cons_rest(t_sfd conn, t_consulta* msg)
{
	t_rta_cons_rest* rta;
	rta = cs_rta_consultar_rest_create(string_array_new());

	// Itera la lista de Restaurantes, agregando los nombres al array
	void _get_restaurantes(ap_restaurante_t* element) {
		string_array_push(&rta->restaurantes, strdup(element->nombre));
	}
	ap_restaurantes_iterate(_get_restaurantes);

	// Si el array está vacío, agrega el Restaurante Default
	if(string_array_is_empty(rta->restaurantes)) {
		string_array_push(&rta->restaurantes, strdup("Default"));
	}

	// Envía la respuesta y finaliza la conexión
	ap_enviar_respuesta(conn, OPCODE_RESPUESTA_OK, CONSULTAR_RESTAURANTES, rta);
	CS_LOG_TRACE("(%d)Se atendió CONSULTAR RESTAURANTES, se cerrará la conexión.", conn);

	close(conn);
	cs_msg_destroy(rta, OPCODE_RESPUESTA_OK, CONSULTAR_RESTAURANTES);
	cs_msg_destroy(msg, OPCODE_CONSULTA, CONSULTAR_RESTAURANTES);
}

static void ap_recibir_sel_rest(t_sfd conn, t_consulta* msg, char* cliente)
{
	if(cliente != NULL)
	{
		//Busca el restaurante en la lista
		int restaurante_index = ap_restaurante_find_index(msg->restaurante);
		if(restaurante_index > 0)
		{
			//Si se encontró, se vincula al cliente
			void _vincular_restaurante(ap_cliente_t* cliente) {
				CS_LOG_TRACE("Se vincularon: %s <-> %s(%d)", cliente->nombre, msg->restaurante, restaurante_index);
				cliente->restaurante_seleccionado = restaurante_index;
			}
			ap_cliente_find(cliente, _vincular_restaurante);

			//Después de vincular, se retorna una RESPUESTA_OK
			ap_enviar_respuesta(conn, OPCODE_RESPUESTA_OK, SELECCIONAR_RESTAURANTE, NULL);
		}
		else
		{
			//Si no está, no se vincula y se retorna una RESPUESTA_FAIL
			CS_LOG_TRACE("No se encontró en la lista de Restaurantes: %s", msg->restaurante);
			ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, SELECCIONAR_RESTAURANTE, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió SELECCIONAR RESTAURANTE, se cerrará la conexión.", conn);
	}
	else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de SELECCIONAR RESTAURANTE!!");
		ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, SELECCIONAR_RESTAURANTE, NULL);
	}

	close(conn);
	cs_msg_destroy(msg, OPCODE_CONSULTA, CONSULTAR_RESTAURANTES);
}

static void ap_recibir_cons_pl(t_sfd conn, t_consulta* msg, char* cliente)
{
	int8_t result;
	t_rta_cons_pl* rta = NULL;
	ap_restaurante_t* restaurante = NULL;

	if(cliente != NULL)
	{
		CS_LOG_TRACE("Buscando al Restaurante vinculado con %s...", cliente);

		//Busca el restaurante a partir del cliente
		void _consultar_platos(ap_restaurante_t* obtenido)
		{
			if(obtenido) {
				restaurante = ap_restaurante_create(
						obtenido->nombre,
						obtenido->posicion,
						obtenido->ip_escucha,
						obtenido->puerto_escucha
				);
			}
		}
		ap_restaurante_get_from_client(cliente, _consultar_platos);

		if(restaurante) //Si se encontró el Restaurante...
		{
			//... y no es Default
			if(strcmp(restaurante->nombre, "Default"))
			{
				CS_LOG_TRACE("Se vinculó: %s <-> %s", cliente, restaurante->nombre);

				//Se envía el mensaje al Restaurante, recibiendo la respuesta
				rta = ap_consultar_restaurante(
						restaurante->ip_escucha,
						restaurante->puerto_escucha,
						CONSULTAR_PLATOS, msg, &result
				);
			}
			else //...y es Default
			{
				CS_LOG_TRACE("No hay restaurantes conectados, se vinculó con Default");

				//Obtiene los platos default
				rta = cs_rta_consultar_pl_create(cs_config_get_string("PLATOS_DEFAULT"));
				result = OPCODE_RESPUESTA_OK;
			}

			//Retorna la respuesta al cliente
			ap_enviar_respuesta(conn, result, CONSULTAR_PLATOS, rta);
			cs_msg_destroy(rta, OPCODE_RESPUESTA_OK, CONSULTAR_PLATOS);
		}
		else //Si NO se encontró el Restaurante retorna RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró ningún Restaurante para: %s", cliente);
			ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, CONSULTAR_PLATOS, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió CONSULTAR PLATOS, se cerrará la conexión.", conn);

	} else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de CONSULTAR PLATOS!!");
		ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, CONSULTAR_PLATOS, NULL);
	}

	close(conn);
	cs_msg_destroy(msg, OPCODE_CONSULTA, CONSULTAR_PLATOS);
	ap_restaurante_destroy(restaurante);
}

static void ap_recibir_crear_ped(t_sfd conn, t_consulta* msg, char* cliente)
{
	int8_t result;
	t_rta_crear_ped* rta = NULL;
	ap_restaurante_t* restaurante = NULL;

	if(cliente != NULL)
	{
		CS_LOG_TRACE("Buscando al Restaurante vinculado con %s...", cliente);

		//Busca el restaurante a partir del cliente
		void _crear_pedido(ap_restaurante_t* obtenido)
		{
			if(obtenido) {
				restaurante = ap_restaurante_create(
						obtenido->nombre,
						obtenido->posicion,
						obtenido->ip_escucha,
						obtenido->puerto_escucha
				);
			}
		}
		ap_restaurante_get_from_client(cliente, _crear_pedido);

		if(restaurante) //Si se encontró el Restaurante...
		{
			if(strcmp(restaurante->nombre, "Default")) //... y no es Default
			{
				CS_LOG_TRACE("Se vinculó: %s <-> %s", cliente, restaurante->nombre);

				//Se envía el mensaje al Restaurante, recibiendo el ID
				rta = ap_consultar_restaurante(
						restaurante->ip_escucha,
						restaurante->puerto_escucha,
						CREAR_PEDIDO, msg, &result
				);
			}
			else //...y es Default
			{
				CS_LOG_TRACE("No hay restaurantes conectados, se vinculó con Default");

				//Se genera un ID único
				pthread_mutex_lock(&mutex_id_default);
				rta = cs_rta_crear_ped_create(++id_default);
				pthread_mutex_unlock(&mutex_id_default);
			}
			//Envía GUARDAR_PEDIDO a la Comanda
			ap_guardar_pedido(restaurante->nombre, msg->pedido_id, &result);

			//Retorna la respuesta al cliente
			ap_enviar_respuesta(conn, result, CREAR_PEDIDO, rta);
			cs_msg_destroy(rta, OPCODE_RESPUESTA_OK, CREAR_PEDIDO);
		}
		else //Si NO se encontró el Restaurante retorna RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró ningún Restaurante para: %s", cliente);
			ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, CREAR_PEDIDO, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió CREAR PEDIDO, se cerrará la conexión.", conn);

	} else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de CREAR PEDIDO!!");
		ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, CREAR_PEDIDO, NULL);
	}

	close(conn);
	cs_msg_destroy(msg, OPCODE_CONSULTA, CREAR_PEDIDO);
	ap_restaurante_destroy(restaurante);
}

static void ap_recibir_aniadir_pl(t_sfd conn, t_consulta* msg, char* cliente)
{
	int8_t result;
	ap_restaurante_t* restaurante = NULL;

	if(cliente != NULL)
	{
		CS_LOG_TRACE("Buscando al Restaurante vinculado con %s...", cliente);

		//Busca el restaurante a partir del cliente
		void _aniadir_plato(ap_restaurante_t* obtenido)
		{
			if(obtenido) {
				restaurante = ap_restaurante_create(
						obtenido->nombre,
						obtenido->posicion,
						obtenido->ip_escucha,
						obtenido->puerto_escucha
				);
			}
		}
		ap_restaurante_get_from_client(cliente, _aniadir_plato);

		if(restaurante) //Si se encontró el Restaurante...
		{
			if(strcmp(restaurante->nombre, "Default")) //... y no es Default
			{
				CS_LOG_TRACE("Se vinculó: %s <-> %s", cliente, restaurante->nombre);

				//Se envía el mensaje al Restaurante, recibiendo solo OK/FAIL
				ap_consultar_restaurante(
						restaurante->ip_escucha,
						restaurante->puerto_escucha,
						ANIADIR_PLATO, msg, &result
				);
			}
			else //...y es Default
			{
				CS_LOG_TRACE("No hay restaurantes conectados, se vinculó con Default");

				//Se omite este paso
				result = OPCODE_RESPUESTA_OK;
			}

			//Si el Restaurante no retornó ningún error, informa a Comanda, guardando el resultado
			if(result == OPCODE_RESPUESTA_OK)
			{
				ap_guardar_plato(msg->comida, restaurante->nombre, msg->pedido_id, &result);
			}

			//Retorna la respuesta al cliente (informando si hubo error o no)
			ap_enviar_respuesta(conn, result, ANIADIR_PLATO, NULL);
		}
		else //Si NO se encontró el Restaurante retorna RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró ningún Restaurante para: %s", cliente);
			ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, ANIADIR_PLATO, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió AÑADIR PLATO, se cerrará la conexión.", conn);

	} else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de ANIADIR PLATO!!");
		ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, ANIADIR_PLATO, NULL);
	}

	close(conn);
	cs_msg_destroy(msg, OPCODE_CONSULTA, ANIADIR_PLATO);
	ap_restaurante_destroy(restaurante);
}

static void ap_recibir_conf_ped(t_sfd conn, t_consulta* msg, char* cliente)
{
	int8_t result;
	ap_restaurante_t* restaurante;

	if(cliente != NULL)
	{
		CS_LOG_TRACE("Buscando al Restaurante vinculado con %s...", cliente);

		//Busca el restaurante a partir del cliente
		void _confirmar_pedido(ap_restaurante_t* obtenido)
		{
			if(obtenido) {
				restaurante = ap_restaurante_create(
						obtenido->nombre,
						obtenido->posicion,
						obtenido->ip_escucha,
						obtenido->puerto_escucha
				);
			}
		}
		ap_restaurante_get_from_client(cliente, _confirmar_pedido);

		if(restaurante) //Si se encontró el Restaurante...
					{
			//Obtiene el pedido desde Comanda, para saber si existe y no se borró de la memoria
			t_rta_obt_ped* pedido = ap_obtener_pedido(restaurante->nombre, msg->pedido_id, &result);
			cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);

			//En caso de existir, evalúa según el Restaurante:
			if(result == OPCODE_RESPUESTA_OK)
			{
				//Si no es Default, reenvía la consulta al Restaurante
				if(strcmp(restaurante->nombre, "Default"))
				{
					CS_LOG_TRACE("Se vinculó: %s <-> %s", cliente, restaurante->nombre);

					//Se envía el mensaje al Restaurante, recibiendo solo OK/FAIL
					ap_consultar_restaurante(
							restaurante->ip_escucha,
							restaurante->puerto_escucha,
							CONFIRMAR_PEDIDO, msg, &result
					);
				}
				else //Si es Default, omite este paso
				{
					CS_LOG_TRACE("No hay restaurantes conectados, se vinculó con Default");

					//Se omite este paso
					result = OPCODE_RESPUESTA_OK;
				}

				//Si el Restaurante no retornó ningún error...
				if(result == OPCODE_RESPUESTA_OK)
				{
					/* Se genera el PCB (Pedido Control Block) del Pedido en cuestión y se deja
					 * en el ciclo de planificación. */
					ap_crear_pcb(cliente, restaurante->nombre, msg->pedido_id);

					//Se informa a Comanda
					ap_confirmar_pedido(restaurante->nombre, msg->pedido_id, &result);
				}
			}

			//Retorna la respuesta al cliente
			ap_enviar_respuesta(conn, result, CONFIRMAR_PEDIDO, NULL);
		}
		else //Si NO se encontró el Restaurante retorna RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró ningún Restaurante para: %s", cliente);
			ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, CONFIRMAR_PEDIDO, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió CONFIRMAR PEDIDO, se cerrará la conexión.", conn);

	} else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de CONFIRMAR PEDIDO!!");
		ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, CONFIRMAR_PEDIDO, NULL);
	}

	close(conn);
	cs_msg_destroy(msg, OPCODE_CONSULTA, CONFIRMAR_PEDIDO);
	ap_restaurante_destroy(restaurante);
}

static void ap_recibir_cons_ped(t_sfd conn, t_consulta* msg, char* cliente)
{
	int8_t result;
	t_rta_cons_ped* rta = NULL;
	ap_restaurante_t* restaurante;

	if(cliente != NULL)
	{
		CS_LOG_TRACE("Buscando al Restaurante vinculado con %s...", cliente);

		//Busca el restaurante a partir del cliente
		void _crear_pedido(ap_restaurante_t* obtenido)
		{
			if(obtenido) {
				restaurante = ap_restaurante_create(
						obtenido->nombre,
						obtenido->posicion,
						obtenido->ip_escucha,
						obtenido->puerto_escucha
				);
			}
		}
		ap_restaurante_get_from_client(cliente, _crear_pedido);

		if(restaurante) //Si se encontró el Restaurante
		{
			if(strcmp(restaurante->nombre, "Default")) {
				CS_LOG_TRACE("Se vinculó: %s <-> %s", cliente, restaurante->nombre);
			} else {
				CS_LOG_TRACE("No hay restaurantes conectados, se vinculó con Default");
			}
			//Obtiene el pedido desde Comanda
			t_rta_obt_ped* pedido = ap_obtener_pedido(restaurante->nombre, msg->pedido_id, &result);

			if(result == OPCODE_RESPUESTA_OK)
			{
				//Crea la respuesta Consultar Pedido
				rta = malloc(sizeof(t_rta_cons_ped));
				rta->restaurante = restaurante->nombre;
				rta->estado_pedido = pedido->estado_pedido;
				rta->platos_y_estados = pedido->platos_y_estados;
			}

			//Retorna la respuesta al cliente
			ap_enviar_respuesta(conn, result, CONSULTAR_PEDIDO, rta);

			cs_msg_destroy(pedido, result, OBTENER_PEDIDO);
			if(result == OPCODE_RESPUESTA_OK) free(rta);
		}
		else //Si NO se encontró el Restaurante retorna RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró ningún Restaurante para: %s", cliente);
			ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, CONSULTAR_PEDIDO, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió CONSULTAR PEDIDO, se cerrará la conexión.", conn);

	} else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de CONSULTAR PEDIDO!!");
		ap_enviar_respuesta(conn, OPCODE_RESPUESTA_FAIL, CONSULTAR_PEDIDO, NULL);
	}

	close(conn);
	cs_msg_destroy(msg, OPCODE_CONSULTA, CONSULTAR_PEDIDO);
	ap_restaurante_destroy(restaurante);
}

static void ap_recibir_pl_listo(t_sfd conn, t_consulta* msg)
{
	printf("Recibí PLATO LISTO\n");

	/* TODO: [APP] Recibir plato listo
	 *
	 * 1. Enviar el mensaje Plato Listo a la Comanda.
	 * 2. Se deberá ejecutar el mensaje Obtener Pedido a la Comanda con el fin de comparar la
	 * cantidad con la cantidad lista. En el caso de que sean iguales, significará que el repartidor
	 * ya puede retirar el pedido del Restaurante, desencadenando los eventos necesarios en la
	 * planificación. */

}

static e_status ap_enviar_respuesta(t_sfd conn, e_opcode op_code, e_msgtype msg_type, void* rta)
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

