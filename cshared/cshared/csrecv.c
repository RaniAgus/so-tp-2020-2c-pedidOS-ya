#include "csrecv.h"

static e_status cs_recv_header(t_sfd sfd_cliente, t_header* header) NON_NULL(2);
static e_status cs_recv_payload(t_sfd sfd_cliente, t_buffer* payload) NON_NULL(2);

e_status cs_recv_msg(t_sfd conn, void (*closure)(t_sfd, t_header, void*))
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

			//Se invoca a la función 'closure' que lo utiliza
			closure(conn, package.header, msg);
		}

		cs_buffer_destroy(package.payload);
	}

	return status;
}

static e_status cs_recv_header(t_sfd conn, t_header* header)
{
	uint32_t bytes;

	bytes = recv(conn, &header->opcode, sizeof(int8_t), MSG_WAITALL);
	if(bytes <= 0) return (bytes == 0) ?
	STATUS_CONN_LOST : ({cs_set_local_err(errno); STATUS_RECV_ERROR;});

	bytes = recv(conn, &header->msgtype, sizeof(int8_t), MSG_WAITALL);
	if(bytes <= 0) return (bytes == 0) ?
	STATUS_CONN_LOST : ({cs_set_local_err(errno); STATUS_RECV_ERROR;});

	return STATUS_SUCCESS;
}

static e_status cs_recv_payload(t_sfd conn, t_buffer* payload)
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

//Todo: [BUFFER -> MSG]

void* cs_buffer_to_msg(t_header header, t_buffer* payload)
{
	switch(header.msgtype)
	{

	default:
		break;
	}
	return NULL;
}
