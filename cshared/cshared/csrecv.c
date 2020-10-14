#include "csrecv.h"
#include "utils/cslog.h"

static e_status cs_recv_header(t_sfd sfd_cliente, t_header* header) NON_NULL(2);
static e_status cs_recv_payload(t_sfd sfd_cliente, t_buffer* payload) NON_NULL(2);
static void* cs_buffer_to_msg(t_header header, t_buffer* payload, t_sfd conn);

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
			msg = cs_buffer_to_msg(package.header, package.payload, conn);

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

	//Recibe el payload
	if(payload->size) {
		payload->stream = malloc(payload->size);
		bytes = recv(conn, payload->stream, payload->size, MSG_WAITALL);
		if(bytes <= 0) {
			return (bytes == 0) ? STATUS_CONN_LOST : ({cs_set_local_err(errno); STATUS_RECV_ERROR;});
		}
	} else {
		payload->stream = NULL;
	}

	return STATUS_SUCCESS;
}

static t_consulta* 		cs_buffer_to_consulta (int8_t msg_type, t_buffer* buffer);
static t_handshake_cli* cs_buffer_to_handshake_cli(t_buffer* buffer);
static t_handshake_res* cs_buffer_to_handshake_res(t_buffer* buffer, t_sfd conn);

static t_rta_handshake_cli*	cs_buffer_to_rta_handshake_cli(t_buffer* buffer);
static t_rta_cons_rest* cs_buffer_to_rta_cons_rest(t_buffer* buffer);
static t_rta_obt_rest*  cs_buffer_to_rta_obt_rest (t_buffer* buffer);
static t_rta_cons_pl*   cs_buffer_to_rta_cons_pl  (t_buffer* buffer);
static t_rta_crear_ped* cs_buffer_to_rta_crear_ped(t_buffer* buffer);
static t_rta_cons_ped*  cs_buffer_to_rta_cons_ped (t_buffer* buffer);
static t_rta_obt_ped*   cs_buffer_to_rta_obt_ped  (t_buffer* buffer);
static t_rta_obt_rec*   cs_buffer_to_rta_obt_rec  (t_buffer* buffer);

static void* cs_buffer_to_msg(t_header header, t_buffer* buffer, t_sfd conn)
{
	switch(header.opcode)
	{
	case OPCODE_CONSULTA:
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE:
			return (void*)cs_buffer_to_handshake_cli(buffer);
		case HANDSHAKE_RESTAURANTE:
			return (void*)cs_buffer_to_handshake_res(buffer, conn);
		default:
			return (void*)cs_buffer_to_consulta(header.msgtype, buffer);
		}
		break;
	case OPCODE_RESPUESTA_OK:
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE:
			return (void*)cs_buffer_to_rta_handshake_cli(buffer);
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

//TODO: cs_buffer_to_consulta -- poner ifs por parámetro, leer módulo desde config
static t_consulta* cs_buffer_to_consulta(int8_t msg_type, t_buffer* buffer)
{
	t_consulta* msg;
	int offset = 0;

	uint32_t comida_len;
	uint32_t restaurante_len;

	int8_t self_module = (int8_t)cs_string_to_enum(cs_config_get_string("MODULO"), cs_enum_module_to_str);

	//El mensaje se puede copiar directamente
	msg = malloc(sizeof(t_consulta));
	msg->msgtype = msg_type;

	//Comida
	if(cs_cons_has_argument(msg_type, CONS_ARG_COMIDA, self_module))
	{
		cs_stream_copy(buffer->stream, &offset, &comida_len      , sizeof(uint32_t), COPY_RECV);
		msg->comida = malloc(comida_len + 1);
		cs_stream_copy(buffer->stream, &offset,  msg->comida     , comida_len      , COPY_RECV);
		msg->comida[comida_len] = '\0';
	} else
	{
		msg->comida = NULL;
	}

	//Cantidad
	if(cs_cons_has_argument(msg_type, CONS_ARG_CANTIDAD, self_module))
	{
		cs_stream_copy(buffer->stream, &offset, &msg->cantidad   , sizeof(uint32_t), COPY_RECV);
	} else
	{
		msg->cantidad = 0;
	}

	//Restaurante
	if(cs_cons_has_argument(msg_type, CONS_ARG_RESTAURANTE, self_module))
	{
		cs_stream_copy(buffer->stream, &offset, &restaurante_len , sizeof(uint32_t), COPY_RECV);
		msg->restaurante = malloc(restaurante_len + 1);
		cs_stream_copy(buffer->stream, &offset,  msg->restaurante, restaurante_len , COPY_RECV);
		msg->restaurante[restaurante_len] = '\0';
	} else
	{
		msg->restaurante = NULL;
	}

	//Pedido id
	if(cs_cons_has_argument(msg_type, CONS_ARG_PEDIDO_ID, self_module))
	{
		cs_stream_copy(buffer->stream, &offset, &msg->pedido_id  , sizeof(uint32_t), COPY_RECV);
	} else
	{
		msg->pedido_id = 0;
	}

	return msg;
}

static t_handshake_cli* cs_buffer_to_handshake_cli(t_buffer* buffer)
{
	t_handshake_cli* msg;
	int offset = 0;

	uint32_t nombre_len;

	//El mensaje se puede copiar directamente
	msg = malloc(sizeof(t_handshake_cli));

	//Nombre
	cs_stream_copy(buffer->stream, &offset, &nombre_len     , sizeof(uint32_t), COPY_RECV);
	msg->nombre = malloc(nombre_len + 1);
	cs_stream_copy(buffer->stream, &offset,  msg->nombre    , nombre_len      , COPY_RECV);
	msg->nombre[nombre_len] = '\0';

	//Posicion
	cs_stream_copy(buffer->stream, &offset, &msg->posicion.x, sizeof(uint32_t), COPY_RECV);
	cs_stream_copy(buffer->stream, &offset, &msg->posicion.y, sizeof(uint32_t), COPY_RECV);

	return msg;
}

static t_handshake_res* cs_buffer_to_handshake_res(t_buffer* buffer, t_sfd conn)
{
	t_handshake_res* msg;
	int offset = 0;

	uint32_t nombre_len, puerto_len;

	//El mensaje se puede copiar directamente
	msg = malloc(sizeof(t_handshake_res));

	//Nombre
	cs_stream_copy(buffer->stream, &offset, &nombre_len     , sizeof(uint32_t), COPY_RECV);
	msg->nombre = malloc(nombre_len + 1);
	cs_stream_copy(buffer->stream, &offset,  msg->nombre    , nombre_len      , COPY_RECV);
	msg->nombre[nombre_len] = '\0';

	//Posicion
	cs_stream_copy(buffer->stream, &offset, &msg->posicion.x, sizeof(uint32_t), COPY_RECV);
	cs_stream_copy(buffer->stream, &offset, &msg->posicion.y, sizeof(uint32_t), COPY_RECV);

	//IP
	cs_get_peer_info(conn, &msg->ip, NULL);

	//Puerto
	cs_stream_copy(buffer->stream, &offset, &puerto_len     , sizeof(uint32_t), COPY_RECV);
	msg->puerto = malloc(puerto_len + 1);
	cs_stream_copy(buffer->stream, &offset,  msg->puerto    , puerto_len      , COPY_RECV);
	msg->puerto[puerto_len] = '\0';

	return msg;
}

static t_rta_cons_rest* cs_buffer_to_rta_cons_rest(t_buffer* buffer)
{
	t_rta_cons_rest* msg;
	int offset = 0;

	char* restaurantes;
	uint32_t restaurantes_len;

	//Restaurantes
	cs_stream_copy(buffer->stream,&offset,&restaurantes_len,sizeof(uint32_t),COPY_RECV);
	restaurantes = malloc(restaurantes_len +1);
	cs_stream_copy(buffer->stream,&offset,restaurantes,restaurantes_len,COPY_RECV);
	restaurantes[restaurantes_len] = '\0';

	//Crea el mensaje
	msg = cs_rta_consultar_rest_create(restaurantes);

	free(restaurantes);

	return msg;
}

static t_rta_obt_rest*  cs_buffer_to_rta_obt_rest(t_buffer* buffer)
{
	t_rta_obt_rest* msg;
	int offset = 0;

	char *afinidades, *comidas, *precios;
	uint32_t afinidades_len, comidas_len, precios_len;
	uint32_t cant_cocineros, cant_hornos, cant_pedidos;
	t_pos pos_restaurante;

	//Cantidad de cocineros
	cs_stream_copy(buffer->stream, &offset, &cant_cocineros   , sizeof(uint32_t), COPY_RECV);

	//Afinidades
	cs_stream_copy(buffer->stream, &offset, &afinidades_len   , sizeof(uint32_t), COPY_RECV);
	afinidades = malloc(afinidades_len + 1);
	cs_stream_copy(buffer->stream, &offset,  afinidades       , afinidades_len  , COPY_RECV);
	afinidades[afinidades_len] = '\0';

	//Menú -- Comidas
	cs_stream_copy(buffer->stream, &offset, &comidas_len      , sizeof(uint32_t), COPY_RECV);
	comidas = malloc(comidas_len + 1);
	cs_stream_copy(buffer->stream, &offset,  comidas          , comidas_len     , COPY_RECV);
	comidas[comidas_len] = '\0';

	//Menú -- Precios
	cs_stream_copy(buffer->stream, &offset, &precios_len      , sizeof(uint32_t), COPY_RECV);
	precios = malloc(precios_len + 1);
	cs_stream_copy(buffer->stream, &offset,  precios          , precios_len     , COPY_RECV);
	precios[precios_len] = '\0';

	//Posición del restaurante
	cs_stream_copy(buffer->stream, &offset, &pos_restaurante.x, sizeof(uint32_t), COPY_RECV);
	cs_stream_copy(buffer->stream, &offset, &pos_restaurante.y, sizeof(uint32_t), COPY_RECV);

	//Cantidad de hornos
	cs_stream_copy(buffer->stream, &offset, &cant_hornos      , sizeof(uint32_t), COPY_RECV);

	//Cantidad de pedidos
	cs_stream_copy(buffer->stream, &offset, &cant_pedidos     , sizeof(uint32_t), COPY_RECV);

	//Crea el mensaje
	msg = cs_rta_obtener_rest_create(cant_cocineros, afinidades, comidas, precios, pos_restaurante, cant_hornos, cant_pedidos);

	free(afinidades);
	free(comidas);
	free(precios);

	return msg;
}

static t_rta_cons_pl*   cs_buffer_to_rta_cons_pl(t_buffer* buffer)
{
	t_rta_cons_pl* msg;
	int offset = 0;

	char *platos;
	uint32_t platos_len;

	//Platos
	cs_stream_copy(buffer->stream,&offset,&platos_len,sizeof(uint32_t),COPY_RECV);
	platos = malloc(platos_len + 1);
	cs_stream_copy(buffer->stream,&offset,platos,platos_len,COPY_RECV);
	platos[platos_len] = '\0';

	//Crea el mensaje
	msg = cs_rta_consultar_pl_create(platos);

	free(platos);

	return msg;
}

static t_rta_crear_ped* cs_buffer_to_rta_crear_ped(t_buffer* buffer)
{
	int offset = 0;

	uint32_t pedido_id;

	//Pedido ID
	cs_stream_copy(buffer->stream,&offset,&pedido_id,sizeof(uint32_t),COPY_RECV);

	return cs_rta_crear_ped_create(pedido_id);
}

static t_rta_cons_ped*  cs_buffer_to_rta_cons_ped(t_buffer* buffer)
{
	t_rta_cons_ped* msg;
	int offset = 0;

	char *restaurante, *comidas, *listos, *totales;
	int8_t estado_pedido;
	uint32_t comidas_len,listos_len,totales_len,restaurante_len;

	//Restaurante
	cs_stream_copy(buffer->stream,&offset,&restaurante_len,sizeof(uint32_t),COPY_RECV);
	restaurante = malloc(restaurante_len + 1);
	cs_stream_copy(buffer->stream,&offset,restaurante,restaurante_len,COPY_RECV);
	restaurante[restaurante_len] = '\0';

	//Estado del pedido
	cs_stream_copy(buffer->stream,&offset,&estado_pedido,sizeof(uint8_t),COPY_RECV);

	//Platos -- Comidas
	cs_stream_copy(buffer->stream,&offset,&comidas_len,sizeof(uint32_t),COPY_RECV);
	comidas = malloc(comidas_len+1);
	cs_stream_copy(buffer->stream,&offset,comidas,comidas_len,COPY_RECV);
	comidas[comidas_len] = '\0';

	//Platos -- Listos
	cs_stream_copy(buffer->stream,&offset,&listos_len,sizeof(uint32_t),COPY_RECV);
	listos = malloc(listos_len+1);
	cs_stream_copy(buffer->stream,&offset,listos,listos_len,COPY_RECV);
	listos[listos_len] = '\0';

	//Platos -- Totales
	cs_stream_copy(buffer->stream,&offset,&totales_len,sizeof(uint32_t),COPY_RECV);
	totales = malloc(totales_len+1);
	cs_stream_copy(buffer->stream,&offset,totales,totales_len,COPY_RECV);
	totales[totales_len] ='\0';

	//Crea el mensaje
	msg = cs_rta_consultar_ped_create(restaurante, (e_estado_ped)estado_pedido, comidas, listos, totales);

	free(restaurante);
	free(comidas);
	free(listos);
	free(totales);

	return msg;
}

static t_rta_obt_ped*   cs_buffer_to_rta_obt_ped(t_buffer* buffer)
{
	t_rta_obt_ped* msg;
	int offset = 0;

	char *comidas, *listos, *totales;
	int8_t estado_pedido;
	uint32_t comidas_len, listos_len, totales_len;

	//Estado del pedido
	cs_stream_copy(buffer->stream,&offset,&estado_pedido,sizeof(uint8_t),COPY_RECV);

	//Comidas
	cs_stream_copy(buffer->stream,&offset,&comidas_len,sizeof(uint32_t),COPY_RECV);
	comidas = malloc(comidas_len + 1);
	cs_stream_copy(buffer->stream,&offset,comidas,comidas_len,COPY_RECV);
	comidas[comidas_len] = '\0';

	//Platos -- Listos
	cs_stream_copy(buffer->stream,&offset,&listos_len,sizeof(uint32_t),COPY_RECV);
	listos = malloc(listos_len + 1);
	cs_stream_copy(buffer->stream,&offset,listos,listos_len,COPY_RECV);
	listos[listos_len] = '\0';

	//Platos -- Totales
	cs_stream_copy(buffer->stream,&offset,&totales_len,sizeof(uint32_t),COPY_RECV);
	totales = malloc(totales_len + 1);
	cs_stream_copy(buffer->stream,&offset,totales,totales_len,COPY_RECV);
	totales[totales_len] ='\0';

	//Crea el mensaje
	msg = cs_rta_obtener_ped_create(estado_pedido, comidas, listos, totales);

	free(comidas);
	free(listos);
	free(totales);

	return msg;
}

static t_rta_obt_rec*   cs_buffer_to_rta_obt_rec(t_buffer* buffer)
{
	t_rta_obt_rec* msg;
	int offset = 0;

	char *pasos, *tiempos;
	uint32_t pasos_len, tiempos_len;

	//Receta -- Pasos
	cs_stream_copy(buffer->stream,&offset,&pasos_len,sizeof(uint32_t),COPY_RECV);
	pasos = malloc(pasos_len + 1);
	cs_stream_copy(buffer->stream,&offset,pasos,pasos_len,COPY_RECV);
	pasos[pasos_len] = '\0';

	//Receta -- Tiempos
	cs_stream_copy(buffer->stream,&offset,&tiempos_len,sizeof(uint32_t),COPY_RECV);
	tiempos = malloc(tiempos_len + 1);
	cs_stream_copy(buffer->stream,&offset,tiempos,tiempos_len,COPY_RECV);
	tiempos[tiempos_len] = '\0';

	//Crea el mensaje
	msg = cs_rta_obtener_receta_create(pasos, tiempos);

	free(pasos);
	free(tiempos);

	return msg;
}

static t_rta_handshake_cli*	cs_buffer_to_rta_handshake_cli(t_buffer* buffer)
{
	t_rta_handshake_cli* msg;
	int offset = 0;

	msg = malloc(sizeof(t_rta_handshake_cli));

	//Módulo (se copia directamente)
	cs_stream_copy(buffer->stream,&offset,&msg->modulo,sizeof(int8_t),COPY_RECV);

	return msg;
}
