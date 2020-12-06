#include "cserror.h"

static const char* CS_STATUS_STR[] =
{
	  "ADDRINFO_ERROR"//	EAI_OVERFLOW = -12
	, "ADDRINFO_ERROR"//	EAI_SYSTEM = -11
	, "ADDRINFO_ERROR"//	EAI_MEMORY = -10
	, "ADDRINFO_ERROR"//
	, "ADDRINFO_ERROR"//	EAI_SERVICE = -8
	, "ADDRINFO_ERROR"//	EAI_SOCKTYPE = -7
	, "ADDRINFO_ERROR"//	EAI_FAMILY = -6
	, "ADDRINFO_ERROR"//
	, "ADDRINFO_ERROR"//	EAI_FAIL = -4
	, "ADDRINFO_ERROR"//	EAI_AGAIN = -3
	, "ADDRINFO_ERROR"//	EAI_NONAME = -2
	, "ADDRINFO_ERROR"//	EAI_BADFLAGS = -1

	, "SUCCESS"
	, "LOOK_UP_ERROR"

	, "SIGACTION_ERROR"

	, "CONFIG_ERROR"
	, "LOGGER_ERROR"

	, "CONN_LOST"
	, "SEND_ERROR"
	, "RECV_ERROR"

	, "GETADDRINFO_ERROR"
	, "SOCKET_ERROR"
	, "BIND_ERROR"
	, "LISTEN_ERROR"
	, "CONNECT_ERROR"
	, "ACCEPT_ERROR"

	, "GETPEERNAME_ERROR"
	, "GETNAMEINFO_ERROR"

	, NULL
};

const char* cs_enum_status_to_str(int value)
{
	return CS_STATUS_STR[value+12];
}

char* cs_string_error(e_status val)
{
	char* str_err;
	if(val < 0)
	{
		str_err = (char*)gai_strerror(val);
	} else
	{
		switch (val)
		{
			case STATUS_SUCCESS:
				str_err = "Success.";
				break;
			case STATUS_CONN_LOST:
				str_err = "Peer has performed an orderly shutdown.";
				break;
			case STATUS_LOGGER_ERROR:
				str_err = errno ? strerror(errno) : "Key not found.";
				break;
			default:
				str_err = errno ? strerror(errno) : "Unknown error.";
				break;
		}
	}

	return str_err;
}
