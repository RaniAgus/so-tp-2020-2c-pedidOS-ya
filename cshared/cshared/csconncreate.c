#include "csconncreate.h"

#define CS_ADDRESS_FAMILY	AF_UNSPEC
#define CS_SOCKET_TYPE		SOCK_STREAM
#define CS_AI_FLAGS			AI_PASSIVE
#define CS_BACKLOG 			SOMAXCONN

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

e_status cs_tcp_server_create(t_sfd* conn, char* ip, char* port)
{
	int err;

	//Parsea las direcciones
	struct addrinfo hints;
	struct addrinfo* serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family   = CS_ADDRESS_FAMILY;
	hints.ai_socktype = CS_SOCKET_TYPE;
	hints.ai_flags    = AI_PASSIVE;

	err = getaddrinfo(ip, port, &hints, &serverInfo);
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
	err = listen(*conn, CS_BACKLOG);
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
	hints.ai_family   = CS_ADDRESS_FAMILY;
	hints.ai_socktype = CS_SOCKET_TYPE;

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
