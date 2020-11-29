#include "sindicato.h"
#include "filesystem.h"

int main(int argc, char* argv[]) {
	for(int i = 1; i<argc; i++) {
		if(string_starts_with(argv[i],"-")) cs_parse_argument(argv[i]);
	}

	sem_init(&bitmapSem, 0, 1);
	tamanioReservado = 4;

	cs_module_init(CONFIG_FILE_PATH, LOG_FILE_KEY, MODULE_NAME);
	leerConfig();

	crearDirectorioAFIP();
	miPuerto = cs_config_get_string("PUERTO_ESCUCHA");
	pthread_t consola;
	pthread_create(&consola, NULL, (void*)atenderConsola, NULL);

	CHECK_STATUS(cs_tcp_server_create(&escucha, miPuerto));

	cs_tcp_server_accept_routine(&escucha, server_recv_msg, server_error_handler);
}

// ----------- INITIALIZER ----------- //

void leerConfig(){
	rutaLog = cs_config_get_string("RUTA_LOG");
	puntoMontaje = cs_config_get_string("PUNTO_MONTAJE");
}

// ----------- ATENDER MENSAJES ----------- //

bool crearRecetaVerificarArgumentos(int argc, char** argv) {
	if(argc < 4){
		CS_LOG_ERROR("Faltan argumentos para crearReceta(3): %d", argc - 1);
		return false;
	}

	if(argc > 4) {
		CS_LOG_ERROR("Sobran argumentos para crearReceta(3): %d", argc - 1);
		return false;
	}

	/* [NOMBRE] */
	if(existeReceta(argv[REC_NOMBRE])) {
		CS_LOG_ERROR("La receta de %s ya existe", argv[REC_NOMBRE]);
		return false;
	}

	// [PASOS]
	if(!cs_string_is_string_array(argv[REC_PASOS])){
		CS_LOG_ERROR("[PASOS] no está en formato array de strings: %s", argv[REC_PASOS]);
		return false;
	}

	// [TIEMPO_PASOS]
	if(!cs_string_is_unsigned_int_array(argv[REC_TIEMPOS])){
		CS_LOG_ERROR("[TIEMPO_PASOS] no está en formato array de enteros: %s", argv[REC_TIEMPOS]);
		return false;
	}

	char** pasos = string_get_string_as_array(argv[REC_PASOS]);
	char** tiempos = string_get_string_as_array(argv[REC_TIEMPOS]);

	// [PASOS] y [TIEMPO_PASOS] no pueden estar vacías
	if(*pasos == NULL || *tiempos == NULL) {
		CS_LOG_ERROR("[PASOS] y [TIEMPO_PASOS] no admiten listas vacias: %s, %s", argv[REC_PASOS], argv[REC_TIEMPOS]);
		liberar_lista(pasos);
		liberar_lista(tiempos);
		return false;
	}

	// [PASOS].size == [TIEMPO_PASOS].size
	if(string_array_size(pasos) != string_array_size(tiempos)) {
		CS_LOG_ERROR("[PASOS] y [TIEMPO_PASOS] no tienen el mismo tamaño: %s, %s", argv[REC_PASOS], argv[REC_TIEMPOS]);
		liberar_lista(pasos);
		liberar_lista(tiempos);
		return false;
	}
	liberar_lista(pasos);
	liberar_lista(tiempos);

	return true;
}

bool crearRestauranteVerificarArgumentos(int argc, char** argv) {
	if(argc < 8){
		CS_LOG_ERROR("Faltan argumentos para crearRestaurante(7)", argc - 1);
		return false;
	}

	if(argc > 8) {
		CS_LOG_ERROR("Sobran argumentos para crearRestaurante(7)", argc - 1);
		return false;
	}

	// [NOMBRE]
	if(existeRestaurante(argv[RES_NOMBRE])) {
		CS_LOG_ERROR("No se pudo crear el restaurante %s porque ya existe", argv[RES_NOMBRE]);
		return false;
	}

	// [CANTIDAD_COCINEROS]
	if(cs_string_to_uint(argv[RES_CANT_COCINEROS]) <= 0) {
		CS_LOG_ERROR("[CANTIDAD_COCINEROS] no está en un formato numérico válido: %s", argv[RES_CANT_COCINEROS]);
		return false;
	}

	// [POSICION]
	if(!cs_string_is_unsigned_int_array(argv[RES_POSICION])){
		CS_LOG_ERROR("[POSICION] no está en formato array de enteros: %s", argv[RES_POSICION]);
		return false;
	}

	// [POSICION].size == 2
	char** posicion = string_get_string_as_array(argv[RES_POSICION]);
	if(string_array_size(posicion) != 2) {
		CS_LOG_ERROR("[POSICION] no está en un formato válido: %s", argv[RES_POSICION]);
		liberar_lista(posicion);
		return false;
	}
	liberar_lista(posicion);

	// [AFINIDAD_COCINEROS]
	if(!cs_string_is_string_array(argv[RES_AFINIDADES])){
		CS_LOG_ERROR("[AFINIDAD_COCINEROS] no está en formato array de strings: %s", argv[RES_AFINIDADES]);
		return false;
	}

	// [CANTIDAD_COCINEROS] >= [AFINIDAD_COCINEROS].size
	char** afinidad_cocineros = string_get_string_as_array(argv[RES_AFINIDADES]);
	if(cs_string_to_uint(argv[RES_CANT_COCINEROS]) < string_array_size(afinidad_cocineros)) {
		CS_LOG_ERROR("[CANTIDAD_COCINEROS] es menor al tamaño de [AFINIDAD_COCINEROS]: %s, %s", argv[RES_CANT_COCINEROS], argv[RES_AFINIDADES]);
		liberar_lista(afinidad_cocineros);
		return false;
	}
	liberar_lista(afinidad_cocineros);

	// [PLATOS]
	if(!cs_string_is_string_array(argv[RES_PLATOS])){
		CS_LOG_ERROR("[PLATOS] no está en formato array de strings: %s", argv[RES_PLATOS]);
		return false;
	}

	// [PRECIO_PLATOS]
	if(!cs_string_is_unsigned_int_array(argv[RES_PRECIOS])){
		CS_LOG_ERROR("[PRECIO_PLATOS] no está en formato array de enteros: %s", argv[RES_PRECIOS]);
		return false;
	}

	char** platos = string_get_string_as_array(argv[RES_PLATOS]);
	char** precio_platos = string_get_string_as_array(argv[RES_PRECIOS]);

	// [PLATOS] y [PRECIO_PLATOS] no pueden estar vacías
	if(*platos == NULL || *precio_platos == NULL) {
		CS_LOG_ERROR("[PLATOS] y [PRECIO_PLATOS] no admiten listas vacias: %s, %s", argv[RES_PLATOS], argv[RES_PRECIOS]);
		liberar_lista(platos);
		liberar_lista(precio_platos);
		return false;
	}

	// [PLATOS].size == [PRECIO_PLATOS].size
	if(string_array_size(platos) != string_array_size(precio_platos)) {
		CS_LOG_ERROR("[PLATOS] y [PRECIO_PLATOS] no tienen el mismo tamaño: %s, %s", argv[RES_PLATOS], argv[RES_PRECIOS]);
		liberar_lista(platos);
		liberar_lista(precio_platos);
		return false;
	}
	liberar_lista(platos);
	liberar_lista(precio_platos);

	// [CANTIDAD_HORNOS]
	if(cs_string_to_uint(argv[RES_CANT_HORNOS]) <= 0) {
		CS_LOG_ERROR("[CANTIDAD_HORNOS] no está en un formato numérico válido: %s", argv[RES_CANT_HORNOS]);
		return false;
	}

	return true;
}

void* atenderConsola(){
	while(1){
		int argc;
		char** argv;
		argv = cs_console_readline(">>> ", &argc);
		if( argc > 0 && (!strcmp(argv[0], "CrearReceta") || !strcmp(argv[0], "CrearRestaurante")) ) {
			if(!strcmp(argv[0], "CrearReceta") && crearRecetaVerificarArgumentos(argc, argv)){
				crearReceta(argv);
			} else if(!strcmp(argv[0], "CrearRestaurante") && crearRestauranteVerificarArgumentos(argc, argv)){
				crearRestaurante(argv);
			}
		} else if(argc > 0){
			CS_LOG_ERROR("Mensaje invalido recibido por consola");
		}
		if(argc > 0) liberar_lista(argv);
	}
}

void server_recv_msg(t_sfd* client_conn){
	e_status status;
	do{
		status = cs_recv_msg(*client_conn, server_log_and_send_reply);
		if( status != STATUS_SUCCESS )	server_error_handler(status);
	} while(status == STATUS_SUCCESS);

	close(*client_conn);
	free((void*)client_conn);
}

// ----------- PIVOT MENSAJES ----------- //

void server_log_and_send_reply(t_sfd client_conn, t_header header, void* msg){
	//Pasa el mensaje a string
	char* msg_str = cs_msg_to_str(msg, header.opcode, header.msgtype);
	t_consulta* elMensaje = (t_consulta*)msg;
	//Loguea el mensaje
	CS_LOG_INFO("%s", msg_str);
	if(header.opcode == OPCODE_CONSULTA)
	{
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE:
			server_send_rta_handshake(client_conn);
			break;
		case GUARDAR_PEDIDO:;
			server_send_rta_ok_fail(header.msgtype, client_conn, guardarPedido(elMensaje));
	    	break;
		case GUARDAR_PLATO:;
			server_send_rta_ok_fail(header.msgtype, client_conn, guardarPlato(elMensaje));
			break;
		case CONFIRMAR_PEDIDO:;
			server_send_rta_ok_fail(header.msgtype, client_conn, confirmarPedido(elMensaje));
			break;
		case PLATO_LISTO:;
			server_send_rta_ok_fail(header.msgtype, client_conn, platoListo(elMensaje));
			break;
		case CONSULTAR_PLATOS:({
			t_header headerResp= {OPCODE_RESPUESTA_OK,CONSULTAR_PLATOS};
			t_rta_cons_pl* respuestaConsultar = consultarPlatos(elMensaje);
			if(!respuestaConsultar){
				server_send_rta_ok_fail(header.msgtype, client_conn,OPCODE_RESPUESTA_FAIL);
			} else{
				char* rta_to_str = cs_msg_to_str(respuestaConsultar, headerResp.opcode, headerResp.msgtype);
				if( cs_send_respuesta(client_conn, headerResp, respuestaConsultar) == STATUS_SUCCESS )
				{
					CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
				} else
				{
					CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
				}
				free(rta_to_str);
				cs_msg_destroy(respuestaConsultar, headerResp.opcode, headerResp.msgtype);
			}
		});
		    break;
		case OBTENER_RESTAURANTE:({
			t_header headerResp= {OPCODE_RESPUESTA_OK,OBTENER_RESTAURANTE};
			t_rta_obt_rest* respuestaObtener = obtenerRestaurante(elMensaje);

			if(!respuestaObtener){
				server_send_rta_ok_fail(header.msgtype, client_conn,OPCODE_RESPUESTA_FAIL);
			} else{
				char* rta_to_str = cs_msg_to_str(respuestaObtener, headerResp.opcode, headerResp.msgtype);
				if( cs_send_respuesta(client_conn, headerResp, respuestaObtener) == STATUS_SUCCESS )
				{
					CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
				} else
				{
					CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
				}
				free(rta_to_str);
				cs_msg_destroy(respuestaObtener, headerResp.opcode, headerResp.msgtype);
			}
		});
			break;
		case OBTENER_PEDIDO:({
			t_header headerResp= {OPCODE_RESPUESTA_OK,OBTENER_PEDIDO};
			t_rta_obt_ped* respuestaObtener = obtenerPedido(elMensaje);

			if(!respuestaObtener){
				server_send_rta_ok_fail(header.msgtype, client_conn,OPCODE_RESPUESTA_FAIL);
			} else{
				char* rta_to_str = cs_msg_to_str(respuestaObtener, headerResp.opcode, headerResp.msgtype);
				if( cs_send_respuesta(client_conn, headerResp, respuestaObtener) == STATUS_SUCCESS )
				{
					CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
				} else
				{
					CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
				}
				free(rta_to_str);
				cs_msg_destroy(respuestaObtener, headerResp.opcode, headerResp.msgtype);
			}
		});
			break;
		case OBTENER_RECETA:({
			t_header headerResp= {OPCODE_RESPUESTA_OK,OBTENER_RECETA};
			t_rta_obt_rec* respuestaObtener = obtenerReceta(elMensaje);

			if(!respuestaObtener){
				server_send_rta_ok_fail(header.msgtype, client_conn,OPCODE_RESPUESTA_FAIL);
			} else{
				char* rta_to_str = cs_msg_to_str(respuestaObtener, headerResp.opcode, headerResp.msgtype);
				if( cs_send_respuesta(client_conn, headerResp, respuestaObtener) == STATUS_SUCCESS )
				{
					CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
				} else
				{
					CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
				}
				free(rta_to_str);
				cs_msg_destroy(respuestaObtener, headerResp.opcode, headerResp.msgtype);
			}
		});
			break;
			case TERMINAR_PEDIDO:;
				server_send_rta_ok_fail(header.msgtype, client_conn, terminarPedido(elMensaje));
			break;
		default:
			server_send_rta_ok_fail(header.msgtype, client_conn,OPCODE_RESPUESTA_FAIL);
		  	break;
		}
	} else {
		puts("algo anda mal xdxd");
	}
	free(msg_str);
	cs_msg_destroy(msg, header.opcode, header.msgtype);
}

// ----------- MANEJO RESPUESTAS ----------- //

void server_error_handler(e_status err){
	CS_LOG_TRACE("%s %d--%s",cs_enum_status_to_str(err), err, cs_string_error(err));
}

void server_send_rta_handshake(t_sfd client_conn)
{
	t_rta_handshake_cli* respuesta;
	char* rta_to_str;
	t_header header = {OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE};
	respuesta = cs_rta_handshake_cli_create();
	rta_to_str = cs_msg_to_str(respuesta, header.opcode, header.msgtype);

	if( cs_send_respuesta(client_conn, header, respuesta) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(respuesta, header.opcode, header.msgtype);
}

void server_send_rta_ok_fail(e_msgtype msg_type, t_sfd client_conn,e_opcode ok_fail)
{
	t_header header= {ok_fail,msg_type};
	char* rta_to_str = cs_msg_to_str(NULL, header.opcode, header.msgtype);
	if( cs_send_respuesta(client_conn, header, NULL) == STATUS_SUCCESS )
	{
		CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
	} else
	{
		CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
	}

	free(rta_to_str);
	cs_msg_destroy(NULL, header.opcode, header.msgtype);
}
