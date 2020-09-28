#include "cssend.h"
#include "utils/cslog.h"

#define CS_STRING_SIZE(str_length)\
	(sizeof(uint32_t) + str_length)

#define CS_HEADER_FIXED_SIZE\
	(sizeof(int8_t) + sizeof(int8_t))

#define CS_PAYLOAD_SIZE\
	(sizeof(uint32_t) + package->payload->size)

static e_status   _send_msg(t_sfd conn, t_header header, t_buffer* (*_to_buffer_func)(void));
static t_package* _package_create(t_header header, t_buffer* payload);
static t_buffer*  _package_to_buffer(t_package* package);
static e_status   _send_all(t_sfd conn, t_buffer* buffer);

e_status cs_send_handshake(t_sfd conn, e_msgtype msg_type, t_handshake* msg)
{
	t_header header = { OPCODE_CONSULTA, msg_type };
	t_buffer* _to_buffer_func(void)
	{
		return cs_handshake_to_buffer(msg);
	}
	return _send_msg(conn, header, _to_buffer_func);
}

e_status cs_send_consulta(t_sfd conn, e_msgtype msg_type, t_consulta* msg, e_module dest)
{
	t_header header = { OPCODE_CONSULTA, msg_type };
	t_buffer* _to_buffer_func(void)
	{
		return cs_consulta_to_buffer(msg, dest);
	}
	return _send_msg(conn, header, _to_buffer_func);
}

e_status cs_send_respuesta(t_sfd conn, t_header header, void* msg)
{
	t_buffer* _to_buffer_func(void)
	{
		return cs_respuesta_to_buffer(header, msg);
	}
	return _send_msg(conn, header, _to_buffer_func);
}

static e_status _send_msg(t_sfd conn, t_header header, t_buffer* (*_to_buffer_func)(void))
{
	e_status status = STATUS_SUCCESS;

	t_buffer  *payload = NULL;
	t_package *package = NULL;
	t_buffer  *buffer  = NULL;

	//Se pasa el mensaje a payload
	payload = _to_buffer_func();

	//Se arma el paquete y se serializa
	package = _package_create(header, payload);
	buffer  = _package_to_buffer(package);

	//Se envía al receptor
	status = _send_all(conn, buffer);

	//Se liberan los recursos
	cs_buffer_destroy(buffer);
	cs_package_destroy(package);
	cs_buffer_destroy(payload);

	return status;
}

static e_status _send_all(t_sfd conn, t_buffer* buffer)
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

static t_package* _package_create(t_header header, t_buffer* payload)
{
	t_package* package;

	package = malloc(sizeof(t_package));

    package->header.opcode  = header.opcode;
    package->header.msgtype = header.msgtype;
    if(payload)
    {
    	package->payload = malloc(sizeof(t_buffer));
    	package->payload->stream = malloc(payload->size);
        memcpy(package->payload->stream, payload->stream, payload->size);
        package->payload->size = payload->size;
    } else
    {
    	package->payload = NULL;
    }

    return package;
}

static t_buffer* _package_to_buffer(t_package* package)
{
	t_buffer* buffer;
	int offset = 0;

	//Se reserva la memoria necesaria
	buffer = malloc(sizeof(t_buffer));

	buffer->size = CS_HEADER_FIXED_SIZE;
	if(package->payload) buffer->size += CS_PAYLOAD_SIZE;
	buffer->stream = malloc(buffer->size);

	cs_stream_copy(buffer->stream, &offset, &package->header.opcode,  sizeof(int8_t), 1);
	cs_stream_copy(buffer->stream, &offset, &package->header.msgtype, sizeof(int8_t), 1);


	cs_stream_copy(buffer->stream, &offset, &package->payload->size,  sizeof(uint32_t), 1);
	cs_stream_copy(buffer->stream, &offset, package->payload->stream, package->payload->size, 1);

	return buffer;
}

static t_buffer* cs_rta_handshake_to_buffer(t_rta_handshake* msg);
static t_buffer* cs_rta_cons_rest_to_buffer(t_rta_cons_rest* msg);
static t_buffer* cs_rta_obt_rest_to_buffer(t_rta_obt_rest* msg);
static t_buffer* cs_rta_cons_pl_to_buffer(t_rta_cons_pl* msg);
static t_buffer* cs_rta_crear_ped_to_buffer(t_rta_crear_ped* msg);
static t_buffer* cs_rta_cons_ped_to_buffer(t_rta_cons_ped* msg);
static t_buffer* cs_rta_obt_ped_to_buffer(t_rta_obt_ped* msg);
static t_buffer* cs_rta_obt_rec_to_buffer(t_rta_obt_rec* msg);

static t_buffer* cs_buffer_create(int size)
{
	t_buffer *buffer = malloc(sizeof(t_buffer));
	if(size > 0) {
		buffer->size = size;
		buffer->stream = malloc(size);
	} else {
		buffer->size = 0;
		buffer->stream = NULL;
	}

	return buffer;
}

//TODO: cs_consulta_to_buffer -- poner ifs por parámetro
t_buffer* cs_consulta_to_buffer(t_consulta* msg, e_module dest)
{
	t_buffer *buffer;
	int offset = 0;

	uint32_t comida_len      = strlen(msg->comida);
	uint32_t restaurante_len = strlen(msg->restaurante);

	buffer = cs_buffer_create(4 * sizeof(uint32_t) + comida_len + restaurante_len);

	cs_stream_copy(buffer->stream, &offset, &comida_len      , sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(buffer->stream, &offset,  msg->comida     , comida_len      , COPY_SEND);
	cs_stream_copy(buffer->stream, &offset, &msg->cantidad   , sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(buffer->stream, &offset, &restaurante_len , sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(buffer->stream, &offset,  msg->restaurante, restaurante_len , COPY_SEND);
	cs_stream_copy(buffer->stream, &offset, &msg->pedido_id  , sizeof(uint32_t), COPY_SEND);

	return buffer;
}

t_buffer* cs_handshake_to_buffer(t_handshake* msg)
{
	t_buffer *buffer;
	int offset = 0;

	uint32_t nombre_len = strlen(msg->nombre);

	buffer = cs_buffer_create(3 * sizeof(uint32_t) + nombre_len);

	cs_stream_copy(buffer->stream, &offset, &nombre_len     , sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(buffer->stream, &offset,  msg->nombre    , nombre_len      , COPY_SEND);
	cs_stream_copy(buffer->stream, &offset, &msg->posicion.x, sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(buffer->stream, &offset, &msg->posicion.y, sizeof(uint32_t), COPY_SEND);

	return buffer;
}

t_buffer* cs_respuesta_to_buffer(t_header header, void* msg)
{
	switch(header.opcode)
	{
	case OPCODE_RESPUESTA_OK:
		switch(header.msgtype)
		{
		case HANDSHAKE:
			return cs_rta_handshake_to_buffer((t_rta_handshake*)msg);
		case CONSULTAR_RESTAURANTES:
			return cs_rta_cons_rest_to_buffer((t_rta_cons_rest*)msg);
		case OBTENER_RESTAURANTE:
			return cs_rta_obt_rest_to_buffer((t_rta_obt_rest*)msg);
		case CONSULTAR_PLATOS:
			return cs_rta_cons_pl_to_buffer((t_rta_cons_pl*)msg);
		case CREAR_PEDIDO:
			return cs_rta_crear_ped_to_buffer((t_rta_crear_ped*)msg);
		case CONSULTAR_PEDIDO:
			return cs_rta_cons_ped_to_buffer((t_rta_cons_ped*)msg);
		case OBTENER_PEDIDO:
			return cs_rta_obt_ped_to_buffer((t_rta_obt_ped*)msg);
		case OBTENER_RECETA:
			return cs_rta_obt_rec_to_buffer((t_rta_obt_rec*)msg);
		default:
			break;
		}
		break;
	default:
		break;
	}
	return cs_buffer_create(0);
}

static t_buffer* cs_rta_cons_rest_to_buffer(t_rta_cons_rest* msg)
{
	t_buffer *buffer;
	int offset = 0;

	char* restaurantes;
	uint32_t restaurantes_len;

	//Convierte los array y listas a string
	restaurantes = cs_string_array_to_string(msg->restaurantes);

	//Calcula la longitud de los strings
	restaurantes_len = strlen(restaurantes);

	buffer = cs_buffer_create(restaurantes_len + sizeof(uint32_t));

	//Restaurantes
	cs_stream_copy(buffer->stream,&offset,&restaurantes_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,restaurantes,restaurantes_len,COPY_SEND);

	free(restaurantes);

	return buffer;
}

static t_buffer* cs_rta_obt_rest_to_buffer(t_rta_obt_rest* msg)
{
	t_buffer *buffer;
	int offset = 0;

	char *afinidades, *comidas, *precios;
	uint32_t afinidades_len, comidas_len, precios_len;

	//Convierte los array y listas a string
	afinidades = cs_string_array_to_string(msg->afinidades);
	cs_menu_to_string(msg->menu, &comidas, &precios);

	//Calcula la longitud de los strings
	afinidades_len = strlen(afinidades);
	comidas_len    = strlen(comidas);
	precios_len    = strlen(precios);

	buffer = cs_buffer_create(8 * sizeof(uint32_t) + afinidades_len + comidas_len + precios_len);

	//Cocineros -- Cantidad (se copia directamente)
	cs_stream_copy(buffer->stream, &offset, &msg->cant_cocineros   , sizeof(uint32_t), COPY_SEND);

	//Cocineros -- Afinidades
	cs_stream_copy(buffer->stream, &offset, &afinidades_len        , sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(buffer->stream, &offset,  afinidades            , afinidades_len  , COPY_SEND);

	//Menú -- Comidas
	cs_stream_copy(buffer->stream, &offset, &comidas_len           , sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(buffer->stream, &offset,  comidas               , comidas_len     , COPY_SEND);

	//Menú -- Precios
	cs_stream_copy(buffer->stream, &offset, &precios_len           , sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(buffer->stream, &offset,  precios               , precios_len     , COPY_SEND);

	//Posición del restaurante (se copia directamente)
	cs_stream_copy(buffer->stream, &offset, &msg->pos_restaurante.x, sizeof(uint32_t), COPY_SEND);
	cs_stream_copy(buffer->stream, &offset, &msg->pos_restaurante.y, sizeof(uint32_t), COPY_SEND);

	//Cantidad de hornos (se copia directamente)
	cs_stream_copy(buffer->stream, &offset, &msg->cant_hornos      , sizeof(uint32_t), COPY_SEND);

	//Cantidad de pedidos
	cs_stream_copy(buffer->stream, &offset, &msg->cant_pedidos     , sizeof(uint32_t), COPY_SEND);

	free(afinidades);
	free(comidas);
	free(precios);

	return buffer;
}

static t_buffer* cs_rta_cons_pl_to_buffer(t_rta_cons_pl* msg)
{
	t_buffer *buffer;
	int offset = 0;

	char* comidas;
	uint32_t comidas_len;

	//Convierte los array y listas a string
	comidas = cs_string_array_to_string(msg->comidas);

	//Calcula la longitud de los strings
	comidas_len = strlen(comidas);

	buffer = cs_buffer_create(comidas_len + sizeof(uint32_t));

	//Comidas
	cs_stream_copy(buffer->stream,&offset,&comidas_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,comidas,comidas_len,COPY_SEND);

	free(comidas);

	return buffer;
}


static t_buffer* cs_rta_crear_ped_to_buffer(t_rta_crear_ped* msg)
{
	t_buffer *buffer;
	int offset = 0;

	buffer = cs_buffer_create(sizeof(uint32_t));

	//Pedido ID
	cs_stream_copy(buffer->stream,&offset,&msg->pedido_id,sizeof(uint32_t),COPY_SEND);

	return buffer;
}

static t_buffer* cs_rta_cons_ped_to_buffer(t_rta_cons_ped* msg)
{
	t_buffer *buffer;
	int offset = 0;

	char *comidas, *listos, *totales;
	uint32_t comidas_len, listos_len, totales_len, restaurante_len;

	//Convierte los array y listas a string
	cs_platos_to_string(msg->platos_y_estados,&comidas,&listos,&totales);

	//Calcula la longitud de los strings
	comidas_len = strlen(comidas);
	listos_len  = strlen(listos);
	totales_len = strlen(totales);
	restaurante_len = strlen(msg->restaurante);

	buffer = cs_buffer_create(4 * sizeof(uint32_t) + comidas_len + listos_len + totales_len + restaurante_len + sizeof(int8_t));

	//Restaurante (se copia directamente)
	cs_stream_copy(buffer->stream,&offset,&restaurante_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,msg->restaurante,restaurante_len,COPY_SEND);

	//Estado del pedido (se copia directamente)
	cs_stream_copy(buffer->stream,&offset,&msg->estado_pedido,sizeof(int8_t),COPY_SEND);

	//Platos -- Comidas
	cs_stream_copy(buffer->stream,&offset,&comidas_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,comidas,comidas_len,COPY_SEND);

	//Platos -- Listos
	cs_stream_copy(buffer->stream,&offset,&listos_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,listos,listos_len,COPY_SEND);

	//Platos -- Totales
	cs_stream_copy(buffer->stream,&offset,&totales_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,totales,totales_len,COPY_SEND);

	free(comidas);
	free(listos);
	free(totales);

	return buffer;
}

static t_buffer* cs_rta_obt_ped_to_buffer(t_rta_obt_ped* msg)
{
	t_buffer *buffer;
	int offset = 0;

	char *comidas, *listos, *totales;
	uint32_t comidas_len,listos_len,totales_len;

	//Convierte los array y listas a string
	cs_platos_to_string(msg->platos_y_estados,&comidas,&listos,&totales);

	//Calcula la longitud de los strings
	comidas_len = strlen(comidas);
	listos_len  = strlen(listos);
	totales_len = strlen(totales);

	buffer = cs_buffer_create(3 * sizeof(uint32_t) + comidas_len + listos_len + totales_len + sizeof(int8_t));

	//Estado del pedido (se copia directamente)
	cs_stream_copy(buffer->stream,&offset,&msg->estado_pedido,sizeof(int8_t),COPY_SEND);

	//Platos -- Comidas
	cs_stream_copy(buffer->stream,&offset,&comidas_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,comidas,comidas_len,COPY_SEND);

	//Platos -- Listos
	cs_stream_copy(buffer->stream,&offset,&listos_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,listos,listos_len,COPY_SEND);

	//Platos -- Totales
	cs_stream_copy(buffer->stream,&offset,&totales_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,totales,totales_len,COPY_SEND);

	free(comidas);
	free(listos);
	free(totales);

	return buffer;
}

static t_buffer* cs_rta_obt_rec_to_buffer(t_rta_obt_rec* msg)
{
	t_buffer *buffer;
	int offset = 0;

	char *pasos, *tiempos;
	uint32_t pasos_len, tiempos_len;

	//Convierte los array y listas a string
	cs_receta_to_string(msg->pasos_receta, &pasos, &tiempos);

	//Calcula la longitud de los strings
	pasos_len   = strlen(pasos);
	tiempos_len = strlen(tiempos);

	buffer = cs_buffer_create(2 * sizeof(uint32_t) + pasos_len + tiempos_len);

	//Receta -- Pasos
	cs_stream_copy(buffer->stream,&offset,&pasos_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,pasos,pasos_len,COPY_SEND);

	//Receta -- Tiempos
	cs_stream_copy(buffer->stream,&offset,&tiempos_len,sizeof(uint32_t),COPY_SEND);
	cs_stream_copy(buffer->stream,&offset,tiempos,tiempos_len,COPY_SEND);

	free(pasos);
	free(tiempos);

	return buffer;
}

//TODO: cs_rta_handshake_to_buffer
static t_buffer* cs_rta_handshake_to_buffer(t_rta_handshake* msg)
{
	return NULL;
}
