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

void cs_package_destroy(t_package* package)
{
	if(package)
	{
		if(package->payload) cs_buffer_destroy(package->payload);
		free(package);
	}

	return;
}

void cs_buffer_destroy(t_buffer* buffer)
{
	if(buffer)
	{
		if(buffer->stream) free(buffer->stream);
		free(buffer);
	}

	return;
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
	    	cs_set_local_err(errno);
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
	struct addrinfo* serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;

	err = getaddrinfo(cs_config_get_string("MI_IP"), port, &hints, &serverInfo);
	if(err)
	{
		freeaddrinfo(serverInfo);
		if (err == EAI_SYSTEM)
		{
			cs_set_local_err(errno); // Algunos errores se setean en 'errno'
			return STATUS_LOOK_UP_ERROR;
		}
		else
		{
			cs_set_local_err(err); // Otros son propios de 'getaddrinfo()' y se leen con 'gai_strerror()'
			return STATUS_GETADDRINFO_ERROR;
		}
	}

	//Obtiene el file descriptor
	*conn = socket(serverInfo->ai_family ,serverInfo->ai_socktype, serverInfo->ai_protocol);
	if(*conn == -1)
	{
		freeaddrinfo(serverInfo);
		cs_set_local_err(errno);
		return STATUS_SOCKET_ERROR;
	}

	//Marca el puerto como reusable
	int yes = 1;
	err = setsockopt(*conn, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if(err == -1)
	{
		freeaddrinfo(serverInfo);
		cs_set_local_err(errno);
		return STATUS_SOCKET_ERROR;
	}

	//Reserva el puerto de escucha
	err = bind(*conn, serverInfo->ai_addr, serverInfo->ai_addrlen);
	if(err == -1)
	{
		freeaddrinfo(serverInfo);
		cs_set_local_err(errno);
		return STATUS_BIND_ERROR;
	}

	//Define el máximo de conexiones pendientes
	err = listen(*conn, SOMAXCONN);
	if(err == -1)
	{
		freeaddrinfo(serverInfo);
		cs_set_local_err(errno);
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
	struct addrinfo *clientInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo(ip, port, &hints, &clientInfo);
	if(err)
	{
		freeaddrinfo(clientInfo);
		if (err == EAI_SYSTEM)
		{
			cs_set_local_err(errno); // Algunos errores se setean en 'errno'
			return STATUS_LOOK_UP_ERROR;
		}
		else
		{
			cs_set_local_err(err); // Otros son propios de 'getaddrinfo()' y se leen con 'gai_strerror()'
			return STATUS_GETADDRINFO_ERROR;
		}
	}

	//Obtiene el file descriptor
	*conn = socket(clientInfo->ai_family,clientInfo->ai_socktype,clientInfo->ai_protocol);
	if(*conn == -1)
	{
		freeaddrinfo(clientInfo);
		cs_set_local_err(errno);
		return STATUS_SOCKET_ERROR;
	}

	//Se conecta al puerto de escucha del servidor
	err = connect(*conn,clientInfo->ai_addr,clientInfo->ai_addrlen);
	if(err == -1)
	{
		freeaddrinfo(clientInfo);
		cs_set_local_err(errno);
		return STATUS_CONNECT_ERROR;
	}

	freeaddrinfo(clientInfo);
	return STATUS_SUCCESS;
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
		cs_set_local_err(errno);
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
		if(err == EAI_SYSTEM)
		{
			cs_set_local_err(errno);
			return STATUS_LOOK_UP_ERROR;
		} else
		{
			cs_set_local_err(err);
			return STATUS_GETADDRINFO_ERROR;
		}
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
