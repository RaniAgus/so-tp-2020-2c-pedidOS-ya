#include "cserror.h"

static int cs_err = 0;
static pthread_mutex_t cs_err_mutex;

static const char* CS_STATUS_STR[] =
{
	"SUCCESS",
	"LOOK_UP_ERROR",

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

void cs_error_init(void)
{
	cs_err = 0;
	pthread_mutex_init(&cs_err_mutex, NULL);
}

void cs_error_delete(void)
{
	pthread_mutex_destroy(&cs_err_mutex);
}

char* cs_string_error(e_status val)
{
	char* str_err;
	switch (val)
	{
		case STATUS_SUCCESS:
			str_err = "Success.";
			break;
		case STATUS_GETADDRINFO_ERROR:
			pthread_mutex_lock(&cs_err_mutex);
			str_err = (char*)gai_strerror(cs_err);
			pthread_mutex_unlock(&cs_err_mutex);
			break;
		case STATUS_REJECTED_MSG:
			str_err = "Message rejected by receiver.";
			break;
		case STATUS_CONN_LOST:
			str_err = "Peer has performed an orderly shutdown.";
			break;
		default:
			pthread_mutex_lock(&cs_err_mutex);
			str_err = ({ cs_err ? strerror(cs_err): "Unknown error.";});
			pthread_mutex_unlock(&cs_err_mutex);
			break;
	}
	return str_err;
}

void cs_set_local_err(int val)
{
	pthread_mutex_lock(&cs_err_mutex);
	cs_err = val;
	pthread_mutex_unlock(&cs_err_mutex);
}
