#include "cserror.h"

static int cs_err = 0;

static const char* CS_STATUS_STR[] =
{
	"SUCCESS",
	"LOOK_UP_ERROR",

	"MALLOC_ERROR",
	"SEM_INIT_ERROR",
	"MUTEX_ERROR",
	"SIGACTION_ERROR",

	"CONFIG_ERROR",
	"LOGGER_ERROR",

	"REJECTED_MSG",
	"CONN_LOST",
	"SEND_ERROR",
	"RECV_ERROR",

	"GETADDRINFO_ERROR",
	"SOCKET_ERROR",
	"BIND_ERROR",
	"LISTEN_ERROR",
	"CONNECT_ERROR",
	"ACCEPT_ERROR",
	"GETPEERNAME_ERROR",

	NULL
};

const char* cs_enum_status_to_str(int value)
{
	return CS_STATUS_STR[value];
}

const char* cs_string_error(e_status val)
{
	switch (val)
	{
		case STATUS_SUCCESS:
			return "Success.";
		case STATUS_GETADDRINFO_ERROR:
			return gai_strerror(cs_err);
		case STATUS_REJECTED_MSG:
			return "Message rejected by receiver.";
		case STATUS_CONN_LOST:
			return "Peer has performed an orderly shutdown.";
		default:
			return strerror(cs_err);
	}
}

void cs_set_local_err(int val)
{
	cs_err = val;
}