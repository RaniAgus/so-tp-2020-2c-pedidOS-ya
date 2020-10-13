#include <cshared/cshared.h>
#include "utilsComanda.h"

#define MODULE_NAME		 "COMANDA"
#define CONFIG_FILE_PATH "comanda.config"
#define LOG_FILE_KEY	 "ARCHIVO_LOG"

static t_sfd conn;
static t_sigaction old_sigint_action;

void server_recv_msg(t_sfd* client_conn);
void server_log_and_send_reply(t_sfd client_conn, t_header header, void* msg);
void server_error_handler(e_status err);

void server_sigint_handler(int signal)
{
	//Cierra la conexión
	close(conn);
	//La marca como '-1' para salir del while (ver cs_tcp_server_accept_routine)
	conn = -1;
	//Devuelve a SIGINT la acción original, por las dudas
	if(sigaction(SIGINT, &old_sigint_action, NULL) == -1)
	{
		server_error_handler(STATUS_SIGACTION_ERROR);
		exit(-1);
	}
}

void server_send_rta_handshake(t_sfd client_conn)
{
	t_rta_handshake* respuesta;
	char* rta_to_str;
	t_header header = {OPCODE_RESPUESTA_OK, HANDSHAKE};
	respuesta = cs_rta_handshake_create();
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



int main(void)
{
	char* str_time;
	//Abre el archivo de configuración
	cs_module_init(CONFIG_FILE_PATH, LOG_FILE_KEY, MODULE_NAME);

	//Modifica la acción de SIGINT para cortar el programa con Ctrl+C
	CHECK_STATUS(cs_signal_change_action(SIGINT,server_sigint_handler,&old_sigint_action));

	//Lee las direcciones desde el config interno
	char* ip   = cs_config_get_string("IP");
	char* port = cs_config_get_string("PUERTO_ESCUCHA");

	//Inicializo memoria
	int tamMemoria= cs_config_get_int("TAMANIO_MEMORIA");
	memoriaPrincipal = malloc(tamMemoria);
	listaRestaurantes = list_create();
	listaFramesMemoria = acomodarFrames(tamMemoria);


	//Abre un socket de escucha 'conn' para aceptar conexiones con 'server_recv_msg'
	CHECK_STATUS(cs_tcp_server_create(&conn, ip, port));

	str_time = cs_temporal_get_string_time("(%d/%m/%y) Servidor abierto a las: %H:%M:%S");
	printf("%s\n[IP: %s] [PUERTO: %s]\n", str_time, ip, port);
	free(str_time);

	cs_tcp_server_accept_routine(&conn, server_recv_msg, server_error_handler);

	str_time = cs_temporal_get_string_time("(%d/%m/%y) Servidor cerrado a las: %H:%M:%S");
	printf("%s\n", str_time);
	free(str_time);

	cs_module_close();

	return EXIT_SUCCESS;
}


//Es la función que se llama al aceptar una conexión 'client_conn' (ver cs_tcp_server_accept_routine)
void server_recv_msg(t_sfd* client_conn)
{
	e_status status;

//Recibe el mensaje del cliente y llama a la función que lo muestra
	do
	{
		status = cs_recv_msg(*client_conn, server_log_and_send_reply);
		if( status != STATUS_SUCCESS )	server_error_handler(status);
	} while(status == STATUS_SUCCESS);

	close(*client_conn);
	free((void*)client_conn);
}

void server_log_and_send_reply(t_sfd client_conn, t_header header, void* msg)
{
	//Pasa el mensaje a string
	char* msg_str = cs_msg_to_str(msg, header.opcode, header.msgtype);
	t_consulta* elMensaje = (t_consulta*)msg;
	//Loguea el mensaje
	CS_LOG_INFO("%s", msg_str);

	switch(header.msgtype)
	{
	case HANDSHAKE:
		server_send_rta_handshake(client_conn);
		break;
	case GUARDAR_PEDIDO:
		puts("guardar pedido");
		e_opcode ok_fail1;
		ok_fail1 = guardarPedido(elMensaje);
		server_send_rta_ok_fail(header.msgtype, client_conn,ok_fail1);
	    break;
	case GUARDAR_PLATO:
		puts("guardar plato");
		e_opcode ok_fail2 = guardarPlato(elMensaje);
		server_send_rta_ok_fail(header.msgtype, client_conn,ok_fail2);
	   	break;
	case OBTENER_PEDIDO:
		puts("obtener pedido");
		//server_send_rta_consultar_platos(client_conn);
	    break;
	case CONFIRMAR_PEDIDO:
		puts("confirmar pedido");
		//server_send_rta_crear_pedido(client_conn);
		break;
	case PLATO_LISTO:
		puts("plato listo");
	    break;
	case FINALIZAR_PEDIDO:
		puts("finalizar pedido");
	    break;
	default:
		puts("algo anda mal xdxd");
	  	break;
	}

	free(msg_str);
	cs_msg_destroy(msg, header.opcode, header.msgtype);
}

void server_error_handler(e_status err)
{
	//Imprime el mensaje de error
	PRINT_ERROR(err);
}





