#include "csrecv.h"
#include "utils/cslog.h"

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
		package.payload = malloc(sizeof(t_buffer));

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
	payload->stream = malloc(payload->size);

	//Recibe el payload
	bytes = recv(conn, payload->stream, payload->size, MSG_WAITALL);
	if(bytes <= 0) return (bytes == 0) ?
	STATUS_CONN_LOST : ({cs_set_local_err(errno); STATUS_RECV_ERROR;});

	return STATUS_SUCCESS;
}

static t_solicitud* 	cs_buffer_to_solicitud    (int8_t msg_type, t_buffer* buffer);
static t_rta_cons_rest* cs_buffer_to_rta_cons_rest(t_buffer* buffer);
static t_rta_obt_rest*  cs_buffer_to_rta_obt_rest (t_buffer* buffer);
static t_rta_cons_pl*   cs_buffer_to_rta_cons_pl  (t_buffer* buffer);
static t_rta_crear_ped* cs_buffer_to_rta_crear_ped(t_buffer* buffer);
static t_rta_cons_ped*  cs_buffer_to_rta_cons_ped (t_buffer* buffer);
static t_rta_obt_ped*   cs_buffer_to_rta_obt_ped  (t_buffer* buffer);
static t_rta_obt_rec*   cs_buffer_to_rta_obt_rec  (t_buffer* buffer);

void* cs_buffer_to_msg(t_header header, t_buffer* buffer)
{
	switch(header.opcode)
	{
	case OPCODE_SOLICITUD:
		return (void*)cs_buffer_to_solicitud(header.msgtype, buffer);
	case OPCODE_RESPUESTA_OK:
		switch(header.msgtype)
		{
		case CONSULTAR_RESTAURANTES:
			return (void*)cs_buffer_to_rta_cons_rest(buffer);
		case OBTENER_RESTAURANTE:
			return (void*)cs_buffer_to_rta_obt_rest(buffer);
		case CONSULTAR_PLATOS:
			return (void*)cs_buffer_to_rta_cons_pl(buffer);
		case CREAR_PEDIDO:
			return (void*)cs_buffer_to_rta_crear_ped(buffer);
		case CONSULTAR_PEDIDO:
			return (void*)cs_buffer_to_rta_cons_ped(buffer);
		case OBTENER_PEDIDO:
			return (void*)cs_buffer_to_rta_obt_ped(buffer);
		case OBTENER_RECETA:
			return (void*)cs_buffer_to_rta_obt_rec(buffer);
		default:
			return NULL;
		}
		break;
	default:
		return NULL;
	}
}

static t_solicitud* cs_buffer_to_solicitud(int8_t msg_type, t_buffer* buffer)
{
	t_solicitud* msg;

	char *comida, *restaurante;
	uint32_t comida_len, restaurante_len, cantidad, pedido_id;

	int offset = 0;

	//Comida
	cs_stream_copy(buffer->stream, &offset, &comida_len     , sizeof(uint32_t), COPY_RECV);
	comida = malloc(comida_len + 1);
	cs_stream_copy(buffer->stream, &offset,  comida         , comida_len      , COPY_RECV);
	comida[comida_len] = '\0';

	//Cantidad
	cs_stream_copy(buffer->stream, &offset, &cantidad       , sizeof(uint32_t), COPY_RECV);

	//Restaurante
	cs_stream_copy(buffer->stream, &offset, &restaurante_len, sizeof(uint32_t), COPY_RECV);
	restaurante = malloc(restaurante_len + 1);
	cs_stream_copy(buffer->stream, &offset,  restaurante    , restaurante_len , COPY_RECV);
	restaurante[restaurante_len] = '\0';

	//Pedido id
	cs_stream_copy(buffer->stream, &offset, &pedido_id      , sizeof(uint32_t), COPY_RECV);

	//Crea el mensaje
	msg = _sol_create(msg_type, comida, cantidad, restaurante, pedido_id);

	//Libera recursos
	free(comida);
	free(restaurante);

	return msg;
}

static t_rta_cons_rest* cs_buffer_to_rta_cons_rest(t_buffer* buffer)
{/*
	char* restaurantes;

	int offset = 0;
*/
	//TODO: [BUFFER->MSG] cs_buffer_to_rta_cons_rest

	return cs_rta_consultar_rest_create(NULL);
}

static t_rta_obt_rest*  cs_buffer_to_rta_obt_rest(t_buffer* buffer)
{
/*
	char *afinidades, *comidas, *precios;
	uint32_t cant_cocineros, cant_hornos;
*/	t_pos pos_restaurante = {0,0};
/*
	int offset = 0;
*/
	//TODO: [BUFFER->MSG] cs_buffer_to_rta_obt_rest

	return cs_rta_obtener_rest_create(0, NULL, NULL, NULL, pos_restaurante, 0);
}

static t_rta_cons_pl*   cs_buffer_to_rta_cons_pl(t_buffer* buffer)
{/*
	char *platos;

	int offset = 0;
*/
	//TODO: [BUFFER->MSG] cs_buffer_to_rta_cons_pl

	return cs_rta_consultar_pl_create(NULL);
}

static t_rta_crear_ped* cs_buffer_to_rta_crear_ped(t_buffer* buffer)
{/*
	uint32_t pedido_id;

	int offset = 0;
*/
	//TODO: [BUFFER->MSG] cs_buffer_to_rta_crear_ped

	return cs_rta_crear_ped_create(0);
}

static t_rta_cons_ped*  cs_buffer_to_rta_cons_ped(t_buffer* buffer)
{/*
	char *rest, *platos, *listos, *totales;
	int8_t estado_ped;

	int offset = 0;
*/
	//TODO: [BUFFER->MSG] cs_buffer_to_rta_cons_ped

	return cs_rta_consultar_ped_create(NULL, 0, NULL, NULL, NULL);
}

static t_rta_obt_ped*   cs_buffer_to_rta_obt_ped(t_buffer* buffer)
{/*
	char *platos, *listos, *totales;

	int offset = 0;
*/
	//TODO: [BUFFER->MSG] cs_buffer_to_rta_obt_ped

	return cs_rta_obtener_ped_create(NULL, NULL, NULL);
}

static t_rta_obt_rec*   cs_buffer_to_rta_obt_rec(t_buffer* buffer)
{/*
	char *pasos, *tiempos;

	int offset = 0;
*/
	//TODO: [BUFFER->MSG] cs_buffer_to_rta_obt_rec

	return cs_rta_obtener_receta_create(NULL, NULL);
}
