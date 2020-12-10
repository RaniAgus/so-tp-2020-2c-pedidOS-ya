#include "apprecepcion.h"

static pthread_t hilo_escucha;
static t_sfd     conexion_escucha;

static uint32_t        id_default;
static pthread_mutex_t mutex_id_default;

static void app_rutina_recepcion_de_mensajes(void);
static void app_recibir_mensaje(t_sfd conexion, t_header header, void* mensaje, char* cliente);

static void app_recibir_handshake_cliente(t_sfd conexion_cliente, t_handshake_cli* handshake_cliente);
static void app_recibir_handshake_restaurante(t_sfd conexion_restaurante, t_handshake_res* handsh);
static void app_recibir_consultar_restaurantes(t_sfd conexion, t_consulta* consulta);
static void app_recibir_seleccionar_restaurante(t_sfd conexion, t_consulta* consulta, char* cliente);
static void app_recibir_consultar_platos(t_sfd conexion, t_consulta* consulta, char* cliente);
static void app_recibir_crear_pedido(t_sfd conexion, t_consulta* consulta, char* cliente);
static void app_recibir_aniadir_plato(t_sfd conexion, t_consulta* consulta, char* cliente);
static void app_recibir_confirmar_pedido(t_sfd conexion, t_consulta* consulta, char* cliente);
static void app_recibir_consultar_pedido(t_sfd conexion, t_consulta* consulta, char* cliente);
static void app_recibir_plato_listo(t_sfd conexion, t_consulta* consulta);

static e_status app_enviar_respuesta(t_sfd conexion, e_opcode op_code, e_msgtype msg_type, void* respuesta);

void app_recepcion_init(void)
{
	t_sfd conexion_comanda;
	e_status status;

	//Se intenta conectar con comanda
	status = cs_tcp_client_create(&conexion_comanda, cs_config_get_string("IP_COMANDA"), cs_config_get_string("PUERTO_COMANDA"));

	if(status == STATUS_SUCCESS) {
		CS_LOG_TRACE("Conectado correctamente con Comanda {IP: %s} {PUERTO: %s}",
				cs_config_get_string("IP_COMANDA"), cs_config_get_string("PUERTO_COMANDA")
		);
		close(conexion_comanda);
	} else {
		CS_LOG_ERROR("%s -- No se pudo conectar con Comanda. Finalizando...", cs_enum_status_to_str(status));
		exit(-1);
	}

	//Inicia el ID de Restaurante Default
	id_default = 0;
	pthread_mutex_init(&mutex_id_default, NULL);

	//Abre un puerto de escucha para recibir mensajes y crea el hilo
	CHECK_STATUS(cs_tcp_server_create(&conexion_escucha, cs_config_get_string("PUERTO_ESCUCHA")));
	pthread_create(&hilo_escucha, NULL, (void*) app_rutina_recepcion_de_mensajes, NULL);
}

// Funciones locales

static void app_rutina_recepcion_de_mensajes(void)
{
	CS_LOG_TRACE("Se abrió un servidor: {PUERTO_ESCUCHA: %s}", cs_config_get_string("PUERTO_ESCUCHA"));

	void _recv_msg(t_sfd* conn) {
		e_status status;

		void _recibir_mensaje(t_sfd conexion, t_header encabezado, void* mensaje) {
			app_recibir_mensaje(conexion, encabezado, mensaje, NULL);
		}

		status = cs_recv_msg(*conn, _recibir_mensaje);
		if(status != STATUS_SUCCESS) PRINT_ERROR(status);

		free((void*)conn);
	}

	void _err_handler(e_status err) { PRINT_ERROR(err); }

	cs_tcp_server_accept_routine(&conexion_escucha, _recv_msg, _err_handler);
}

static void app_recibir_mensaje(t_sfd conexion, t_header header, void* mensaje, char* cliente)
{
	char *mensaje_string = cs_msg_to_str(mensaje, header.opcode, header.msgtype);
	CS_LOG_TRACE("(%d)Se recibió el mensaje: %s", conexion, mensaje_string);
	free(mensaje_string);

	if(header.opcode == OPCODE_CONSULTA) //Si es consulta, la atiende
	{
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE:       app_recibir_handshake_cliente      (conexion, mensaje); break;
		case HANDSHAKE_RESTAURANTE:   app_recibir_handshake_restaurante  (conexion, mensaje); break;
		case CONSULTAR_RESTAURANTES:  app_recibir_consultar_restaurantes (conexion, mensaje); break;
		case SELECCIONAR_RESTAURANTE: app_recibir_seleccionar_restaurante(conexion, mensaje, cliente); break;
		case CONSULTAR_PLATOS:        app_recibir_consultar_platos       (conexion, mensaje, cliente); break;
		case CREAR_PEDIDO:            app_recibir_crear_pedido           (conexion, mensaje, cliente); break;
		case ANIADIR_PLATO:           app_recibir_aniadir_plato          (conexion, mensaje, cliente); break;
		case CONFIRMAR_PEDIDO:        app_recibir_confirmar_pedido       (conexion, mensaje, cliente); break;
		case CONSULTAR_PEDIDO:        app_recibir_consultar_pedido       (conexion, mensaje, cliente); break;
		case PLATO_LISTO:             app_recibir_plato_listo            (conexion, mensaje); break;
		default:
			/* La App NO recibe:
			 * OBTENER RESTAURANTE
			 * GUARDAR/OBTENER/TERMINAR/FINALIZAR PEDIDO
			 * GUARDAR PLATO
			 * OBTENER RECETA */
			app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, header.msgtype, NULL);
			break;
		}
		//Si se recibió desde cliente, se cierra la conexión
		if(cliente != NULL){
			close(conexion);
			CS_LOG_TRACE("(%d)Se cerró la conexión.", conexion);
		}
	} else //Si no es consulta, loguea un error
	{
		CS_LOG_ERROR("Se esperaba una consulta!!");
		close(conexion);
	}
}

static void app_recibir_handshake_cliente(t_sfd conexion_cliente, t_handshake_cli* handshake_cliente)
{
	e_status status;
	t_rta_handshake_cli* respuesta;

	//Crea una respuesta handshake
	respuesta = cs_rta_handshake_cli_create();

	// Si el cliente no se encuentra, se agrega
	if(!app_cliente_esta_conectado(handshake_cliente->nombre))
	{
		CS_LOG_TRACE("(%d)No se encontró a %s entre la lista de Clientes, se agregará a la misma.",
				conexion_cliente, handshake_cliente->nombre
		);
		app_conectar_cliente(handshake_cliente->nombre, handshake_cliente->posicion, conexion_cliente);

		//Después de agregar, se envía la RESPUESTA_OK
		app_enviar_respuesta(conexion_cliente, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE, respuesta);
	}
	else // Si se encuentra, primero hay que enviar RESPUESTA_OK para recibir la (única) consulta
	{
		CS_LOG_TRACE("(%d)Se encontró a %s entre la lista de Clientes, se recibirá la consulta.",
				conexion_cliente, handshake_cliente->nombre
		);
		if( app_enviar_respuesta(conexion_cliente, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE, respuesta) == STATUS_SUCCESS )
		{
			void _recibir_consulta(t_sfd conexion, t_header header, void* consulta) {
				//Llama la misma función que recibió el handshake, pero con el nombre del Cliente como parámetro
				app_recibir_mensaje(conexion, header, consulta, handshake_cliente->nombre);
			}
			status = cs_recv_msg(conexion_cliente, _recibir_consulta);
			if(status != STATUS_SUCCESS) {
				PRINT_ERROR(status);
			}
		}
	}

	cs_msg_destroy(handshake_cliente, OPCODE_CONSULTA, HANDSHAKE_CLIENTE);
	cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE);
}

static void app_recibir_handshake_restaurante(t_sfd conexion_restaurante, t_handshake_res* handsh)
{
	e_status status;

	//Se agrega a la lista de restaurantes y se envía RESPUESTA_OK
	app_conectar_restaurante(handsh->nombre, handsh->posicion, handsh->ip, handsh->puerto);
	CS_LOG_TRACE("Se agregó el Restaurante: {NOMBRE: %s} {POS: (%d,%d)} {IP_PUERTO: %s:%s}",
			handsh->nombre, handsh->posicion.x, handsh->posicion.y, handsh->ip, handsh->puerto
	);

	status = app_enviar_respuesta(conexion_restaurante, OPCODE_RESPUESTA_OK, HANDSHAKE_RESTAURANTE, NULL);

	//Si se envió correctamente, recibe mensajes PLATO_LISTO del Restaurante por este socket
	CS_LOG_TRACE("(%d)Inició la conexión con el Restaurante.", conexion_restaurante);
	while(status == STATUS_SUCCESS)
	{
		void _recibir_consultas(t_sfd conexion, t_header encabezado, void* consulta) {
			//Llama la misma función que recibió el handshake
			app_recibir_mensaje(conexion, encabezado, consulta, NULL);
		}
		status = cs_recv_msg(conexion_restaurante, _recibir_consultas);
		if(status != STATUS_SUCCESS) {
			PRINT_ERROR(status);
		}
	}
	CS_LOG_TRACE("(%d)Se cerró la conexión con el Restaurante.", conexion_restaurante);

	//Se desvinculan todos los clientes del Restaurante
	app_desconectar_restaurante(handsh->nombre);
	cs_msg_destroy(handsh, OPCODE_CONSULTA, HANDSHAKE_RESTAURANTE);

	//Se cierra el socket después de atender las consultas del Restaurante
	close(conexion_restaurante);
}

static void app_recibir_consultar_restaurantes(t_sfd conexion, t_consulta* consulta)
{
	t_rta_cons_rest* rta;
	rta = cs_rta_consultar_rest_create(string_array_new());

	// Itera la lista de Restaurantes, agregando los nombres al array
	void _get_restaurantes(char* nombre, app_restaurante_t* _) {
		string_array_push(&rta->restaurantes, strdup(nombre));
	}
	app_iterar_restaurantes(_get_restaurantes);

	// Si el array está vacío, agrega el Restaurante Default
	if(string_array_is_empty(rta->restaurantes)) {
		string_array_push(&rta->restaurantes, strdup("Default"));
	}

	// Envía la respuesta y finaliza la conexión
	app_enviar_respuesta(conexion, OPCODE_RESPUESTA_OK, CONSULTAR_RESTAURANTES, rta);
	CS_LOG_TRACE("(%d)Se atendió CONSULTAR RESTAURANTES, se cerrará la conexión.", conexion);

	cs_msg_destroy(rta, OPCODE_RESPUESTA_OK, CONSULTAR_RESTAURANTES);
	cs_msg_destroy(consulta, OPCODE_CONSULTA, CONSULTAR_RESTAURANTES);
}

static void app_recibir_seleccionar_restaurante(t_sfd conexion, t_consulta* consulta, char* cliente)
{
	//Se tiene que haber identificado el Cliente enviando un HANDSHAKE_CLIENTE primero!
	if(cliente != NULL)
	{
		//Busca si el restaurante está conectado
		if(app_restaurante_esta_conectado(consulta->restaurante))
		{
			app_cliente_t* info_cliente = app_obtener_cliente(cliente);
			pthread_mutex_lock(&info_cliente->mutex_rest_vinculado);
			if(info_cliente->rest_vinculado != NULL) {
				CS_LOG_TRACE("Se desvincularon: %s <-> %s", cliente, info_cliente->rest_vinculado);
				free(info_cliente->rest_vinculado);
			}
			info_cliente->rest_vinculado = string_duplicate(consulta->restaurante);
			CS_LOG_TRACE("Se vincularon: %s <-> %s", cliente, info_cliente->rest_vinculado);
			pthread_mutex_unlock(&info_cliente->mutex_rest_vinculado);

			//Después de vincular, se retorna una RESPUESTA_OK
			app_enviar_respuesta(conexion, OPCODE_RESPUESTA_OK, SELECCIONAR_RESTAURANTE, NULL);
		}
		else //Si no se encontró, se retorna una RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró en la lista de Restaurantes: %s", consulta->restaurante);
			app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, SELECCIONAR_RESTAURANTE, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió SELECCIONAR RESTAURANTE, se cerrará la conexión.", conexion);
	}
	else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de SELECCIONAR RESTAURANTE!!");
		app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, SELECCIONAR_RESTAURANTE, NULL);
	}

	cs_msg_destroy(consulta, OPCODE_CONSULTA, CONSULTAR_RESTAURANTES);
}

static void app_recibir_consultar_platos(t_sfd conexion, t_consulta* consulta, char* cliente)
{
	int8_t result; //Todos los "result" son enum opcode, pueden ser RESPUESTA_OK/FAIL
	t_rta_cons_pl* respuesta = NULL;

	if(cliente != NULL)
	{
		CS_LOG_TRACE("Buscando al Restaurante vinculado con %s...", cliente);

		//Busca el restaurante a partir del cliente
		char* restaurante_vinculado = app_obtener_restaurante_vinculado_a_cliente(cliente);

		if(restaurante_vinculado != NULL)
		{
			// Si se encontró el restaurante y NO es Default, se envía el mensaje al Restaurante
			// y se obtiene la respuesta
			if(!string_equals_ignore_case(restaurante_vinculado, "Default"))
			{
				CS_LOG_TRACE("Se vinculó: %s <-> %s", cliente, restaurante_vinculado);
				respuesta = app_consultar_restaurante(restaurante_vinculado, CONSULTAR_PLATOS, consulta, &result);
			}
			else //Si es Default, se obtienen los platos desde config
			{
				CS_LOG_TRACE("No hay restaurantes conectados, se vinculó con Default");

				respuesta = cs_rta_consultar_pl_create(cs_config_get_string("PLATOS_DEFAULT"));
				result = OPCODE_RESPUESTA_OK;
			}

			//Retorna la respuesta al cliente
			app_enviar_respuesta(conexion, result, CONSULTAR_PLATOS, respuesta);
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, CONSULTAR_PLATOS);
		}
		else //Si NO se encontró el Restaurante retorna RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró ningún Restaurante para: %s", cliente);
			app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, CONSULTAR_PLATOS, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió CONSULTAR PLATOS, se cerrará la conexión.", conexion);
		free(restaurante_vinculado);
	} else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de CONSULTAR PLATOS!!");
		app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, CONSULTAR_PLATOS, NULL);
	}

	cs_msg_destroy(consulta, OPCODE_CONSULTA, CONSULTAR_PLATOS);
}

static void app_recibir_crear_pedido(t_sfd conexion, t_consulta* consulta, char* cliente)
{
	int8_t result;
	t_rta_crear_ped* respuesta = NULL;

	if(cliente != NULL)
	{
		CS_LOG_TRACE("Buscando al Restaurante vinculado con %s...", cliente);

		//Busca el restaurante a partir del cliente
		char* restaurante_vinculado = app_obtener_restaurante_vinculado_a_cliente(cliente);

		if(restaurante_vinculado)
		{
			if(!string_equals_ignore_case(restaurante_vinculado, "Default"))
			{
				CS_LOG_TRACE("Se vinculó: %s <-> %s", cliente, restaurante_vinculado);

				//Si se encontró un Restaurante no-Default, se envía el mensaje al Restaurante, recibiendo el ID
				respuesta = app_consultar_restaurante(restaurante_vinculado, CREAR_PEDIDO, consulta, &result);
			}
			else //Si se encontró el Default, se genera un ID único
			{
				CS_LOG_TRACE("No hay restaurantes conectados, se vinculó con Default");

				pthread_mutex_lock(&mutex_id_default);
				respuesta = cs_rta_crear_ped_create(++id_default);
				pthread_mutex_unlock(&mutex_id_default);
			}
			//Envía GUARDAR_PEDIDO a la Comanda
			app_guardar_pedido(restaurante_vinculado, respuesta->pedido_id, &result);

			//Retorna la respuesta al cliente
			app_enviar_respuesta(conexion, result, CREAR_PEDIDO, respuesta);
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, CREAR_PEDIDO);
		}
		else //Si NO se encontró el Restaurante retorna RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró ningún Restaurante para: %s", cliente);
			app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, CREAR_PEDIDO, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió CREAR PEDIDO, se cerrará la conexión.", conexion);
		free(restaurante_vinculado);

	} else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de CREAR PEDIDO!!");
		app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, CREAR_PEDIDO, NULL);
	}

	cs_msg_destroy(consulta, OPCODE_CONSULTA, CREAR_PEDIDO);
}

static void app_recibir_aniadir_plato(t_sfd conexion, t_consulta* consulta, char* cliente)
{
	int8_t result;

	if(cliente != NULL)
	{
		CS_LOG_TRACE("Buscando al Restaurante vinculado con %s...", cliente);

		//Busca el restaurante a partir del cliente
		char* restaurante_vinculado = app_obtener_restaurante_vinculado_a_cliente(cliente);

		if(restaurante_vinculado != NULL)
		{
			//Si se encontró el Restaurante y no es Default:
			if(!string_equals_ignore_case(restaurante_vinculado, "Default"))
			{
				CS_LOG_TRACE("Se vinculó: %s <-> %s", cliente, restaurante_vinculado);
				
				//Se envía el mensaje al Restaurante, recibiendo solo OK/FAIL
				app_consultar_restaurante(restaurante_vinculado, ANIADIR_PLATO, consulta, &result);
			}
			else //Si se encontró y es Default, se omite este paso
			{
				CS_LOG_TRACE("No hay restaurantes conectados, se vinculó con Default");
				result = OPCODE_RESPUESTA_OK;
			}

			//Si el Restaurante no retornó ningún error, informa a Comanda, guardando el resultado
			if(result == OPCODE_RESPUESTA_OK)
			{
				app_guardar_plato(consulta->comida, restaurante_vinculado, consulta->pedido_id, &result);
			}

			//Retorna la respuesta al cliente (informando si hubo error o no)
			app_enviar_respuesta(conexion, result, ANIADIR_PLATO, NULL);
		}
		else //Si NO se encontró el Restaurante retorna RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró ningún Restaurante para: %s", cliente);
			app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, ANIADIR_PLATO, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió AÑADIR PLATO, se cerrará la conexión.", conexion);
		free(restaurante_vinculado);
	} else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de ANIADIR PLATO!!");
		app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, ANIADIR_PLATO, NULL);
	}

	cs_msg_destroy(consulta, OPCODE_CONSULTA, ANIADIR_PLATO);
}

static void app_recibir_confirmar_pedido(t_sfd conexion, t_consulta* consulta, char* cliente)
{
	int8_t result;

	if(cliente != NULL)
	{
		CS_LOG_TRACE("Buscando al Restaurante vinculado con %s...", cliente);

		//Busca el restaurante a partir del cliente
		char* restaurante_vinculado = app_obtener_restaurante_vinculado_a_cliente(cliente);

		if(restaurante_vinculado != NULL) //Si se encontró el Restaurante:
		{
			//Obtiene el pedido desde Comanda, para saber si existe y no se borró de la memoria
			t_rta_obt_ped* pedido = app_obtener_pedido(restaurante_vinculado, consulta->pedido_id, &result);
			cs_msg_destroy(pedido, result, OBTENER_PEDIDO);

			//En caso de existir, evalúa según el Restaurante:
			if(result == OPCODE_RESPUESTA_OK)
			{
				//Si no es Default, reenvía la consulta al Restaurante
				if(!string_equals_ignore_case(restaurante_vinculado, "Default"))
				{
					CS_LOG_TRACE("Se vinculó: %s <-> %s", cliente, restaurante_vinculado);

					//Se envía el mensaje al Restaurante, recibiendo solo OK/FAIL
					app_consultar_restaurante(restaurante_vinculado, CONFIRMAR_PEDIDO, consulta, &result);
				}
				else //Si es Default, omite este paso
				{
					CS_LOG_TRACE("No hay restaurantes conectados, se vinculó con Default");

					//Se omite este paso
					result = OPCODE_RESPUESTA_OK;
				}

				//Si el Restaurante no retornó ningún error:
				if(result == OPCODE_RESPUESTA_OK)
				{
					/* Se genera el PCB (Pedido Control Block) del Pedido en cuestión y se deja
					 * en el ciclo de planificación. */
					app_crear_pcb(cliente, restaurante_vinculado, consulta->pedido_id);

					//Se informa a Comanda
					app_confirmar_pedido(restaurante_vinculado, consulta->pedido_id, &result);
				}
			}

			//Retorna la respuesta al cliente
			app_enviar_respuesta(conexion, result, CONFIRMAR_PEDIDO, NULL);
		}
		else //Si NO se encontró el Restaurante retorna RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró ningún Restaurante para: %s", cliente);
			app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, CONFIRMAR_PEDIDO, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió CONFIRMAR PEDIDO, se cerrará la conexión.", conexion);
		free(restaurante_vinculado);
	} else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de CONFIRMAR PEDIDO!!");
		app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, CONFIRMAR_PEDIDO, NULL);
	}

	cs_msg_destroy(consulta, OPCODE_CONSULTA, CONFIRMAR_PEDIDO);
}

static void app_recibir_consultar_pedido(t_sfd conexion, t_consulta* consulta, char* cliente)
{
	int8_t result;
	t_rta_cons_ped* respuesta = NULL;

	if(cliente != NULL)
	{
		CS_LOG_TRACE("Buscando al Restaurante vinculado con %s...", cliente);

		//Busca el restaurante a partir del cliente
		char* restaurante_vinculado = app_obtener_restaurante_vinculado_a_cliente(cliente);

		if(restaurante_vinculado != NULL) //Si se encontró el Restaurante
		{
			if(!string_equals_ignore_case(restaurante_vinculado, "Default")) {
				CS_LOG_TRACE("Se vinculó: %s <-> %s", cliente, restaurante_vinculado);
			} else {
				CS_LOG_TRACE("No hay restaurantes conectados, se vinculó con Default");
			}

			//Obtiene el pedido desde Comanda
			t_rta_obt_ped* pedido = app_obtener_pedido(restaurante_vinculado, consulta->pedido_id, &result);

			if(result == OPCODE_RESPUESTA_OK)
			{
				//Crea la respuesta Consultar Pedido
				respuesta = malloc(sizeof(t_rta_cons_ped));
				respuesta->restaurante = restaurante_vinculado;
				respuesta->estado_pedido = pedido->estado_pedido;
				respuesta->platos_y_estados = pedido->platos_y_estados;
			}

			//Retorna la respuesta al cliente
			app_enviar_respuesta(conexion, result, CONSULTAR_PEDIDO, respuesta);

			cs_msg_destroy(pedido, result, OBTENER_PEDIDO);
			if(result == OPCODE_RESPUESTA_OK) free(respuesta);
		}
		else //Si NO se encontró el Restaurante retorna RESPUESTA_FAIL
		{
			CS_LOG_TRACE("No se encontró ningún Restaurante para: %s", cliente);
			app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, CONSULTAR_PEDIDO, NULL);
		}

		CS_LOG_TRACE("(%d)Se atendió CONSULTAR PEDIDO, se cerrará la conexión.", conexion);
		free(restaurante_vinculado);
	} else
	{
		CS_LOG_ERROR("Falta identificar al cliente antes de CONSULTAR PEDIDO!!");
		app_enviar_respuesta(conexion, OPCODE_RESPUESTA_FAIL, CONSULTAR_PEDIDO, NULL);
	}

	cs_msg_destroy(consulta, OPCODE_CONSULTA, CONSULTAR_PEDIDO);
}

static void app_recibir_plato_listo(t_sfd conexion, t_consulta* consulta)
{
	int8_t result;

	//Envía el mensaje Plato Listo a la Comanda
	app_plato_listo(consulta, &result);
	if(result == OPCODE_RESPUESTA_OK)
	{
		//Obtiene el pedido desde Comanda
		t_rta_obt_ped* pedido = app_obtener_pedido(consulta->restaurante, consulta->pedido_id, &result);

		//Se fija si todos los platos están listos
		if(cs_platos_estan_listos(pedido->platos_y_estados)) {
			app_avisar_pedido_terminado(consulta->restaurante, consulta->pedido_id);
		} else {
			CS_LOG_TRACE("El pedido NO está terminado, no se dará ningún aviso al repartidor.");
		}

		cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
	}

	//Retorna la respuesta
	app_enviar_respuesta(conexion, result, PLATO_LISTO, NULL);

	CS_LOG_TRACE("(%d)Se atendió PLATO LISTO.", conexion);
	cs_msg_destroy(consulta, OPCODE_CONSULTA, CONSULTAR_PEDIDO);
}

static e_status app_enviar_respuesta(t_sfd conexion, e_opcode op_code, e_msgtype msg_type, void* respuesta)
{
	e_status status;
	t_header header = { op_code, msg_type };
	char* respuesta_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	status = cs_send_respuesta(conexion, header, respuesta);
	if(status == STATUS_SUCCESS)
	{
		CS_LOG_TRACE("Se envió la respuesta: %s", respuesta_str);
	}
	else
	{
		CS_LOG_ERROR("%s -- No se pudo enviar la respuesta: %s", cs_enum_status_to_str(status), respuesta_str);
	}

	free(respuesta_str);
	return status;
}

