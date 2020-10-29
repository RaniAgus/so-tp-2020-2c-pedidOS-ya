#include "sindicato.h"
#include "filesystem.h"

int main(void) {

	sem_init(&bitmapSem, 0, 1);

	cs_module_init(CONFIG_FILE_PATH, LOG_FILE_KEY, MODULE_NAME);
	leerConfig();

	if(!strcmp(magicNumber, "AFIP")){
		crearDirectorioAFIP();
	}
	consultarPlatos("Maxi");

	miPuerto = cs_config_get_string("PUERTO_ESCUCHA");

	pthread_t consola;
	pthread_create(&consola, NULL, (void*)atenderConsola, NULL);

	CHECK_STATUS(cs_tcp_server_create(&escucha, miPuerto));

	cs_tcp_server_accept_routine(&escucha, server_recv_msg, server_error_handler);
}

// ----------- INITIALIZER ----------- //

void leerConfig(){
<<<<<<< HEAD
	rutaLog = cs_config_get_string("RUTA_LOG");
	magicNumber = cs_config_get_string("MAGIC_NUMBER");
	puntoMontaje = cs_config_get_string("PUNTO_MONTAJE_TALLGRASS");
	tamanioBloque = cs_config_get_int("BLOCKS_SIZE");
	cantidadBloques = cs_config_get_int("BLOCKS");

}

// ----------- ATENDER MENSAJES ----------- //

void* atenderConsola(){
	while(1){
		int argc;
		char** argv;
		argv = cs_console_readline(">", &argc);
		if(argc==4 && !strcmp(argv[0], "CrearReceta") && cs_string_is_string_array(argv[2]) && cs_string_is_unsigned_int_array(argv[3])){
			crearReceta(argv);
			continue;
		}
		if(argc==8 && !strcmp(argv[0], "CrearRestaurante") && cs_string_to_uint(argv[2]) > 0 && cs_string_is_unsigned_int_array(argv[3]) &&
			cs_string_is_string_array(argv[4]) && cs_string_is_string_array(argv[5]) && cs_string_is_unsigned_int_array(argv[6]) &&
			cs_string_to_uint(argv[7]) > 0 ){
			crearRestaurante(argv);
			continue;
		}
		CS_LOG_TRACE("Mensaje invalido recibido por consola");
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
		case GUARDAR_PEDIDO:({
			e_opcode ok_fail1;
			ok_fail1 = guardarPedido(elMensaje);
			server_send_rta_ok_fail(header.msgtype, client_conn,ok_fail1);
		});
		    break;
		case GUARDAR_PLATO:({
			e_opcode ok_fail1;
			ok_fail1 = guardarPlato(elMensaje);
			server_send_rta_ok_fail(header.msgtype, client_conn,ok_fail1);
		});
			break;
		case CONFIRMAR_PEDIDO:({
			e_opcode ok_fail1;
			ok_fail1 = confirmarPedido(elMensaje);
			server_send_rta_ok_fail(header.msgtype, client_conn,ok_fail1);
		});
			break;
		case PLATO_LISTO:({
			e_opcode ok_fail1;
			ok_fail1 = platoListo(elMensaje);
			server_send_rta_ok_fail(header.msgtype, client_conn,ok_fail1);
		});
			break;
		case CONSULTAR_PLATOS:({
			t_header headerResp= {OPCODE_RESPUESTA_OK,OBTENER_PEDIDO};
			t_rta_cons_pl* respuestaObtener = consultarPlatos(elMensaje);
=======
//	config = config_create("gamecard.config");
//	ip = config_get_string_value(config,"IP_BROKER");
>>>>>>> master

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
			}
		});
		    break;
		case OBTENER_RESTAURANTE:({
			t_header headerResp= {OPCODE_RESPUESTA_OK,OBTENER_PEDIDO};
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
			}
		});
			break;
		case OBTENER_PEDIDO:({
			t_header headerResp= {OPCODE_RESPUESTA_OK,OBTENER_PEDIDO};
			t_rta_obt_ped* respuestaObtener = obtenerRestaurante(elMensaje);

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
			}
		});
			break;
		case OBTENER_RECETA:({
			t_header headerResp= {OPCODE_RESPUESTA_OK,OBTENER_PEDIDO};
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
			}
		});
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





