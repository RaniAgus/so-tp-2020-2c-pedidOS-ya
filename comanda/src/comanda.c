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

int main(void)
{
	//Abre el archivo de configuración
	cs_module_init(CONFIG_FILE_PATH, LOG_FILE_KEY, MODULE_NAME);

	//Modifica la acción de SIGINT para cortar el programa con Ctrl+C
	CHECK_STATUS(cs_signal_change_action(SIGINT,server_sigint_handler,&old_sigint_action));

	//Lee las direcciones desde el config interno
	char* ip   = cs_config_get_string("IP");
	char* port = cs_config_get_string("PUERTO_ESCUCHA");

	//Abre un socket de escucha 'conn' para aceptar conexiones con 'server_recv_msg'
	CHECK_STATUS(cs_tcp_server_create(&conn, ip, port));

	cs_temporal_do(LAMBDA(void, (char* date, char* time),{
		printf("(%s) Servidor abierto a las: %s\n", date, time);
	}));

	cs_tcp_server_accept_routine(&conn, server_recv_msg, server_error_handler);

	cs_temporal_do(LAMBDA(void, (char* date, char* time),{
		printf("(%s) Servidor cerrado a las: %s\n", date, time);
	}));

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
	case GUARDAR_PEDIDO:
	//	server_send_rta_consultar_restaurantes(client_conn);
	    break;
	case GUARDAR_PLATO:
	//	server_send_rta_obtener_restaurante(client_conn);
	   	break;
	case OBTENER_PEDIDO:
		//server_send_rta_consultar_platos(client_conn);
	    break;
	case CONFIRMAR_PEDIDO:
		//server_send_rta_crear_pedido(client_conn);
		break;
	case PLATO_LISTO:
	    break;
	case FINALIZAR_PEDIDO:
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
