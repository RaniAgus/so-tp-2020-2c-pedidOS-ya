#include "cssend.h"

#define CS_STRING_SIZE(str_length)\
	(sizeof(uint32_t) + str_length)

//TODO: [HEADER]: Tamaño del header
#define CS_HEADER_FIXED_SIZE\
	(sizeof(int8_t) + sizeof(int8_t) + sizeof(uint32_t))

#define CS_PAYLOAD_SIZE\
	(sizeof(uint32_t) + package->payload->size)

//TODO: [MSG]: Tamaño de cada tipo de mensaje
#define CS_MSG_NEW_SIZE(str_length)\
	(CS_STRING_SIZE(str_length) + 2* sizeof(uint32_t) + sizeof(uint32_t))

static t_package* cs_package_create(t_header header, t_buffer* payload);
static t_buffer*  cs_package_to_buffer(t_package* package);

e_status cs_connect_and_send_msg(const char* ip_key, const char* port_key,
									t_header header, void* msg, uint32_t* ack_ptr)
{
	e_status status;
	t_buffer* payload = NULL;
	t_sfd conn = -1;

	//Lee las direcciones desde el config interno
	char* ip   = cs_config_get_string(ip_key);
	char* port = cs_config_get_string(port_key);

	//Se conecta al receptor como cliente
	status = cs_tcp_client_create(&conn, ip, port);
	if(status != STATUS_SUCCESS) return status;


	//Se pasa el mensaje a payload
	payload = cs_msg_to_buffer(header, msg);

	//Se envía al receptor
	status = cs_send(conn, header, payload);
	if(status == STATUS_SUCCESS)
	{
		//Se espera respuesta ack
		status = cs_recv_ack(conn, ack_ptr);
		if(status == STATUS_SUCCESS && *ack_ptr == 0)
		{
			//Si el receptor devuelve ack=0, es porque el mensaje fue rechazado
			status = STATUS_REJECTED_MSG;
		}
	}

	//Se liberan los recursos
	cs_buffer_destroy(payload);
	close(conn);

	return status;
}

e_status cs_send(t_sfd conn, t_header header, t_buffer* payload)
{
	e_status status = STATUS_SUCCESS;

	t_package* package = NULL;
	t_buffer*  buffer  = NULL;

	uint32_t bytes_sent = 0;
	uint32_t bytes_left;
	uint32_t n;

	//Se arma el paquete y se serializa
	package = cs_package_create(header, payload);
	buffer  = cs_package_to_buffer(package);

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

	//Se liberan los recursos
	cs_package_destroy(package);
	cs_buffer_destroy(buffer);

	return status;
}

e_status cs_recv_ack(t_sfd conn, uint32_t* ack_ptr)
{
	ssize_t bytes;

	bytes = recv(conn, ack_ptr, sizeof(uint32_t), MSG_WAITALL);

	if(bytes <= 0) return (bytes == 0) ?
	STATUS_CONN_LOST : ({cs_set_local_err(errno); STATUS_RECV_ERROR;});

	return STATUS_SUCCESS;
}

static t_package* cs_package_create(t_header header, t_buffer* payload)
{
	t_package* package;

	CHECK_STATUS(MALLOC(package, sizeof(t_package)));

	//TODO: [HEADER]: Agregar al paquete
    package->header.opcode  = header.opcode;
    package->header.msgtype = header.msgtype;
    package->header.msg_id  = header.msg_id;
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

	//TODO: [HEADER]: Serializar
	cs_stream_copy(buffer->stream, &offset, &package->header.opcode,  sizeof(int8_t), 1);
	cs_stream_copy(buffer->stream, &offset, &package->header.msgtype, sizeof(int8_t), 1);
	cs_stream_copy(buffer->stream, &offset, &package->header.msg_id, sizeof(uint32_t), 1);

	//El mensaje a enviar puede no requerir de payload (ej: una suscripción)
	if(package->payload)
	{
		cs_stream_copy(buffer->stream, &offset, &package->payload->size,  sizeof(uint32_t), 1);
		cs_stream_copy(buffer->stream, &offset, package->payload->stream, package->payload->size, 1);
	}

	return buffer;
}

//Todo: [MSG -> BUFFER]
static t_buffer* cs_msg_new_to_buffer(t_msg_new* msg);

t_buffer* cs_msg_to_buffer(t_header header, void* msg)
{
	switch(header.msgtype)
	{
	case NEW_POKEMON:
		return cs_msg_new_to_buffer((t_msg_new*)msg);
	}
	return NULL;
}

static t_buffer* cs_msg_new_to_buffer(t_msg_new* msg)
{
	t_buffer* payload = NULL;
	uint32_t str_length = 0;
	int offset = 0;

	//Los strings se van a enviar sin el '\0' final (puede cambiarse)
	str_length = (uint32_t)strlen(msg->name);

	//Se reserva la memoria y se guarda el tamaño del payload
	CHECK_STATUS(MALLOC(payload, sizeof(t_buffer)));
	payload->size = CS_MSG_NEW_SIZE(str_length);
	CHECK_STATUS(MALLOC(payload->stream, payload->size));

	//Copia cada parte del mensaje al stream
	cs_stream_copy(payload->stream, &offset, &str_length, sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(payload->stream, &offset, msg->name  , str_length      , COPY_SEND);
	cs_stream_copy(payload->stream, &offset, &msg->pos.x, sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(payload->stream, &offset, &msg->pos.y, sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(payload->stream, &offset, &msg->cant , sizeof(uint32_t), COPY_SEND);

	return payload;
}

