#include "cssend.h"

#define CS_STRING_SIZE(str_length)\
	(sizeof(uint32_t) + str_length)

#define CS_HEADER_FIXED_SIZE\
	(sizeof(int8_t) + sizeof(int8_t))

#define CS_PAYLOAD_SIZE\
	(sizeof(uint32_t) + package->payload->size)

//TODO: [MSG]: Tamaño de cada tipo de mensaje
#define CS_MSG_NEW_SIZE(str_length)\
	(CS_STRING_SIZE(str_length) + 2* sizeof(uint32_t) + sizeof(uint32_t))

static e_status   cs_send_all(t_sfd conn, t_buffer* buffer);
static t_package* cs_package_create(t_header header, t_buffer* payload);
static t_buffer*  cs_package_to_buffer(t_package* package);

e_status cs_send_msg(t_sfd conn, t_header header, void* msg)
{
	e_status status;

	t_buffer*  payload = NULL;
	t_package* package = NULL;
	t_buffer*  buffer  = NULL;

	//Se pasa el mensaje a payload
	payload = cs_msg_to_buffer(header, msg);

	//Se arma el paquete y se serializa
	package = cs_package_create(header, payload);
	buffer  = cs_package_to_buffer(package);

	//Se envía al receptor
	status = cs_send_all(conn, buffer);

	//Se liberan los recursos
	cs_buffer_destroy(buffer);
	cs_package_destroy(package);
	cs_buffer_destroy(payload);

	return status;
}

static e_status cs_send_all(t_sfd conn, t_buffer* buffer)
{
	e_status status = STATUS_SUCCESS;

	uint32_t bytes_sent = 0;
	uint32_t bytes_left;
	uint32_t n;

	bytes_left = buffer->size;
	do
	{	//'MSG_NOSIGNAL' para que no se envíe la señal 'SIGPIPE' al destinatario
		n = send(conn, (buffer->stream) + bytes_sent, bytes_left, MSG_NOSIGNAL);
		if(n == -1)
		{
			cs_set_local_err(errno);
			if(errno == EPIPE)
				status = STATUS_CONN_LOST;
			else
				status = STATUS_SEND_ERROR;
		} else
		{
			bytes_sent += n;
			bytes_left -= n;
		}
	//El protocolo TCP puede partir el paquete si es muy grande (>1K)
	} while(bytes_sent < buffer->size && status == STATUS_SUCCESS);

	return status;
}

static t_package* cs_package_create(t_header header, t_buffer* payload)
{
	t_package* package;

	CHECK_STATUS(MALLOC(package, sizeof(t_package)));

    package->header.opcode  = header.opcode;
    package->header.msgtype = header.msgtype;
    if(payload)
    {
    	CHECK_STATUS(MALLOC(package->payload, sizeof(t_buffer)));
    	CHECK_STATUS(MALLOC(package->payload->stream, payload->size));
        memcpy(package->payload->stream, payload->stream, payload->size);
        package->payload->size = payload->size;
    } else
    {
    	package->payload = NULL;
    }

    return package;
}

static t_buffer* cs_package_to_buffer(t_package* package)
{
	t_buffer* buffer;
	int offset = 0;

	//Se reserva la memoria necesaria
	CHECK_STATUS(MALLOC(buffer, sizeof(t_buffer)));

	buffer->size = CS_HEADER_FIXED_SIZE;
	if(package->payload) buffer->size += CS_PAYLOAD_SIZE;
	CHECK_STATUS(MALLOC(buffer->stream, buffer->size));

	cs_stream_copy(buffer->stream, &offset, &package->header.opcode,  sizeof(int8_t), 1);
	cs_stream_copy(buffer->stream, &offset, &package->header.msgtype, sizeof(int8_t), 1);

	//El mensaje a enviar puede no requerir de payload (ej: una suscripción)
	if(package->payload)
	{
		cs_stream_copy(buffer->stream, &offset, &package->payload->size,  sizeof(uint32_t), 1);
		cs_stream_copy(buffer->stream, &offset, package->payload->stream, package->payload->size, 1);
	}

	return buffer;
}

//Todo: [MSG -> BUFFER]
t_buffer* cs_msg_to_buffer(t_header header, void* msg)
{
	switch(header.msgtype)
	{
	default:
		break;
	}
	return NULL;
}

