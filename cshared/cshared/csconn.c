#include "csconn.h"

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

e_status cs_get_peer_info(t_sfd sfd, char** ip_ptr, char** port_ptr)
{
	char *ip, *port;

	struct sockaddr_storage addr;
	socklen_t addr_size = sizeof(struct sockaddr_storage);

	ip   = strdup("0000:0000:0000:0000:0000:0000:0000:0000");
	port = strdup("65535");

	if(getpeername(sfd, (struct sockaddr *)&addr, &addr_size) == -1)
	{
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
