#include "csrecv.h"

e_status cs_recv_msg(t_sfd conn, uint32_t (*closure)(t_header, void*))
{
	e_status status;
	t_package package;
	void* msg;

	//Se recibe el header
	status = cs_recv_header(conn, &package.header);
	if(status == STATUS_SUCCESS)
	{
		CHECK_STATUS(MALLOC(package.payload, sizeof(t_buffer)));

		//Se recibe el payload
		status = cs_recv_payload(conn, package.payload);
		if(status == STATUS_SUCCESS)
		{
			//Se pasa el payload a mensaje
			msg = cs_buffer_to_msg(package.header, package.payload);
			//Se invoca a la función 'closure' que lo utiliza, la cual retorna el valor del ack a devolver
			status = cs_send_ack(conn, closure(package.header, msg));
		}

		cs_buffer_destroy(package.payload);
	}

	return status;
}

e_status cs_recv_header(t_sfd conn, t_header* header)
{
	uint32_t bytes;

	//TODO: [HEADER]: Recibir
	bytes = recv(conn, &header->opcode, sizeof(int8_t), MSG_WAITALL);
	if(bytes <= 0) return (bytes == 0) ?
	STATUS_CONN_LOST : ({cs_set_local_err(errno); STATUS_RECV_ERROR;});

	bytes = recv(conn, &header->msgtype, sizeof(int8_t), MSG_WAITALL);
	if(bytes <= 0) return (bytes == 0) ?
	STATUS_CONN_LOST : ({cs_set_local_err(errno); STATUS_RECV_ERROR;});

	bytes = recv(conn, &header->msg_id, sizeof(uint32_t), MSG_WAITALL);
	if(bytes <= 0) return (bytes == 0) ?
	STATUS_CONN_LOST : ({cs_set_local_err(errno); STATUS_RECV_ERROR;});

	return STATUS_SUCCESS;
}

e_status cs_recv_payload(t_sfd conn, t_buffer* payload)
{
	uint32_t bytes;

	//Recibe el size del payload
	bytes = recv(conn, &payload->size, sizeof(uint32_t), MSG_WAITALL);
	if(bytes <= 0) return (bytes == 0) ?
	STATUS_CONN_LOST : ({cs_set_local_err(errno); STATUS_RECV_ERROR;});

	//Reserva la memoria necesaria
	CHECK_STATUS(MALLOC(payload->stream, payload->size));

	//Recibe el payload
	bytes = recv(conn, payload->stream, payload->size, MSG_WAITALL);
	if(bytes <= 0) return (bytes == 0) ?
	STATUS_CONN_LOST : ({cs_set_local_err(errno); STATUS_RECV_ERROR;});

	return STATUS_SUCCESS;
}

e_status   cs_send_ack(e_status conn, uint32_t ack)
{
	e_status status = STATUS_SUCCESS;

	int	sent;

	//'MSG_NOSIGNAL' para que no se envíe la señal 'SIGPIPE' al destinatario
	sent = send(conn, &ack, sizeof(uint32_t), MSG_NOSIGNAL);
	if(sent == -1)
	{
		cs_set_local_err(errno);
		if(errno == EPIPE)
			status = STATUS_CONN_LOST; //Se utilizó 'shutdown()' del otro lado
		else
			status = STATUS_SEND_ERROR;
	}

	return status;
}

//Todo: [BUFFER -> MSG]
static t_msg_new* cs_buffer_to_msg_new(t_buffer* payload);

void* cs_buffer_to_msg(t_header header, t_buffer* payload)
{
	switch(header.msgtype)
	{
	case NEW_POKEMON:
		return (void*)cs_buffer_to_msg_new(payload);
	default:
		break;
	}
	return NULL;
}

static t_msg_new* cs_buffer_to_msg_new(t_buffer* payload)
{
	t_msg_new* msg = NULL;
	uint32_t str_length = 0;
	int offset = 0;

	CHECK_STATUS(MALLOC(msg, sizeof(t_msg_new)));

	cs_stream_copy(payload->stream, &offset, &str_length, sizeof(uint32_t), COPY_RECV);

	//Los strings se envían sin el '\0' final (puede cambiarse)
	CHECK_STATUS(MALLOC(msg->name, str_length + 1));
	msg->name[str_length] = '\0';

	//Copia cada parte del mensaje desde el stream
	cs_stream_copy(payload->stream, &offset, msg->name  , str_length      , COPY_RECV);
	cs_stream_copy(payload->stream, &offset, &msg->pos.x, sizeof(uint32_t), COPY_RECV);
	cs_stream_copy(payload->stream, &offset, &msg->pos.y, sizeof(uint32_t), COPY_RECV);
	cs_stream_copy(payload->stream, &offset, &msg->cant , sizeof(uint32_t), COPY_RECV);

	return msg;
}

