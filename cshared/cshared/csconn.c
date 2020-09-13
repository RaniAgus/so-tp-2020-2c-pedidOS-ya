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

e_status cs_get_peer_info(t_sfd sfd, char** ip_str_ptr, char** port_str_ptr)
{
	int err;

	struct sockaddr_storage addr;
	socklen_t addr_size = sizeof(struct sockaddr_storage);

	*ip_str_ptr   = string_new();
	*port_str_ptr = string_new();

	char* ip_str   = string_duplicate("0000:0000:0000:0000:0000:0000:0000:0000");
	char* port_str = string_duplicate("65535");

	err = getpeername(sfd, (struct sockaddr *)&addr, &addr_size);
	if(err == -1)
	{
		cs_set_local_err(errno);
		return STATUS_GETPEERNAME_ERROR;
	}

	err = getnameinfo((struct sockaddr *)&addr, addr_size,
			ip_str, strlen(ip_str) + 1,
			port_str, strlen(port_str) + 1, 0);
	if(err != 0)
	{
		printf("ERROR: %d\n", err);
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

	if(ip_str_ptr)   string_append(ip_str_ptr, ip_str);
	if(port_str_ptr) string_append(port_str_ptr, port_str);

	free(ip_str);
	free(port_str);

	return STATUS_SUCCESS;
}
