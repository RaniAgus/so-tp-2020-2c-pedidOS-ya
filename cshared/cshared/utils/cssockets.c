#include "cssockets.h"

static const char* CS_OPCODE_STR[] =
{
	"UNKNOWN",
	"MENSAJE",
	"RESPUESTA_OK",
	"RESPUESTA_FAIL",
	NULL
};

const char* cs_enum_opcode_to_str(int value)
{
	return CS_OPCODE_STR[value];
}

void cs_tcp_server_accept_routine(t_sfd* conn, void(*success_action)(t_sfd*), void(*err_handler)(e_status))
{
	t_sfd* client_conn;

	struct sockaddr_storage client_addr;
	unsigned int addr_size = sizeof(struct sockaddr_storage);

	while(*conn != -1)
	{
		client_conn  = malloc(sizeof(t_sfd));
	    *client_conn = accept(*conn,(struct sockaddr*) &client_addr, &addr_size);
	    if(*client_conn == -1)
	    {
	    	free((void*)client_conn);
	    	err_handler(STATUS_ACCEPT_ERROR);
	    	continue;
	    }
	    pthread_t client_thread;
	    pthread_create(&client_thread, NULL, (void*)success_action, (void*)client_conn);
	    pthread_detach(client_thread);
	}
}

e_status cs_tcp_server_create(t_sfd* conn, char* port)
{
	int err;

	//Parsea las direcciones
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;

	err = getaddrinfo(NULL, port, &hints, &serverInfo);
	if(err)
	{
		freeaddrinfo(serverInfo);
		// Algunos errores se setean en 'errno' y otros son propios de 'getaddrinfo()' y se leen con 'gai_strerror()'
		return err == EAI_SYSTEM ? STATUS_GETADDRINFO_ERROR : err;
	}

	//Obtiene el file descriptor
	*conn = socket(serverInfo->ai_family ,serverInfo->ai_socktype, serverInfo->ai_protocol);
	if(*conn == -1)
	{
		freeaddrinfo(serverInfo);
		return STATUS_SOCKET_ERROR;
	}

	//Marca el puerto como reusable
	int yes = 1;
	err = setsockopt(*conn, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if(err == -1)
	{
		freeaddrinfo(serverInfo);
		return STATUS_SOCKET_ERROR;
	}

	//Reserva el puerto de escucha
	err = bind(*conn, serverInfo->ai_addr, serverInfo->ai_addrlen);
	if(err == -1)
	{
		freeaddrinfo(serverInfo);
		return STATUS_BIND_ERROR;
	}

	//Define el máximo de conexiones pendientes
	err = listen(*conn, SOMAXCONN);
	if(err == -1)
	{
		freeaddrinfo(serverInfo);
		return STATUS_LISTEN_ERROR;
	}

	freeaddrinfo(serverInfo);
	return STATUS_SUCCESS;
}

e_status cs_tcp_client_create(t_sfd* conn, char* ip, char* port)
{
	int err;

	//Parsea las direcciones
	struct addrinfo hints;
	struct addrinfo *clientInfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo(ip, port, &hints, &clientInfo);
	if(err)
	{
		freeaddrinfo(clientInfo);
		// Algunos errores se setean en 'errno' y otros son propios de 'getaddrinfo()' y se leen con 'gai_strerror()'
		return err == EAI_SYSTEM ? STATUS_GETADDRINFO_ERROR : err;
	}

	for(p = clientInfo; p != NULL; p = p->ai_next)
	{
		//Obtiene el file descriptor
		*conn = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(*conn == -1)
			continue;

		//Se conecta al puerto de escucha del servidor
		err = connect(*conn, p->ai_addr, p->ai_addrlen);
		if(err != -1)
			break;

		close(*conn);
	}

	freeaddrinfo(clientInfo);

	return *conn == -1 ? STATUS_SOCKET_ERROR : err == -1 ? STATUS_CONNECT_ERROR : STATUS_SUCCESS;
}

e_status cs_send_all(t_sfd conn, t_buffer* buffer)
{
	t_buffer* temp = buffer_create();
	buffer_pack(temp, &buffer->size, sizeof(uint32_t));
	buffer_pack(temp, buffer->stream, buffer->size);

	int bytes_sent = 0;
	int bytes_left = temp->size;

	//El protocolo TCP puede partir el paquete si es muy grande (>1K)
	while(bytes_left > 0)
	{
		//'MSG_NOSIGNAL' para que no se envíe la señal 'SIGPIPE' al destinatario
		int n = send(conn, temp->stream + bytes_sent, bytes_left, MSG_NOSIGNAL);
		if(n == -1)
		{
			buffer_destroy(temp);
			if(errno == EPIPE)
				return STATUS_CONN_LOST;
			else
				return STATUS_SEND_ERROR;
		} else
		{
			bytes_sent += n;
			bytes_left -= n;
		}
	}

	buffer_destroy(temp);
	return STATUS_SUCCESS;
}

e_status cs_receive_all(t_sfd sockfd, t_buffer* buffer)
{
	t_buffer temp;
	int bytes_received;

	bytes_received = recv(sockfd, &temp.size, sizeof(uint32_t), MSG_WAITALL);
	if(bytes_received > 0)
	{
		temp.stream = malloc(temp.size);
		bytes_received = recv(sockfd, temp.stream, temp.size, MSG_WAITALL);
		if(bytes_received > 0)
		{
			buffer_pack(buffer, temp.stream, temp.size);
		}
		free(temp.stream);
	}

	return bytes_received > 0 ? STATUS_SUCCESS : (bytes_received == 0 ?  STATUS_CONN_LOST : STATUS_RECV_ERROR);
}

e_status cs_get_peer_info(t_sfd sfd, char** ip_ptr, char** port_ptr)
{
	char *ip, *port;

	struct sockaddr_storage addr;
	socklen_t addr_size = sizeof(struct sockaddr_storage);

	ip   = strdup("0000:0000:0000:0000:0000:0000:0000:0000");
	port = strdup("65535");

	if(getpeername(sfd, (struct sockaddr *)&addr, &addr_size) == -1)
	{
		free(ip);
		free(port);
		return STATUS_GETPEERNAME_ERROR;
	}

	int err = getnameinfo(
			(struct sockaddr *)&addr, addr_size,
			ip, strlen(ip) + 1,
			port, strlen(port) + 1, 0
	);
	if(err != 0)
	{
		free(ip);
		free(port);
		// Algunos errores se setean en 'errno' y otros son propios de 'getnameinfo()' y se leen con 'gai_strerror()'
		return err == EAI_SYSTEM ? STATUS_GETNAMEINFO_ERROR : err;
	}

	if(ip_ptr)   *ip_ptr   = strdup(ip);
	if(port_ptr) *port_ptr = strdup(port);

	free(ip);
	free(port);

	return STATUS_SUCCESS;
}

bool cs_socket_is_connected(t_sfd sfd)
{
	bool is_connected;

	void* stream = malloc(1);
	is_connected = recv(sfd, stream, 1, MSG_PEEK | MSG_DONTWAIT) != 0;
	free(stream);

	return is_connected;
}
