#include "csrecv.h"

e_status cs_recv_msg(t_sfd conn, void (*closure)(t_sfd, t_header, void*))
{
	e_status status;

	t_header header;
	t_buffer* buffer = buffer_create();

	status = cs_receive_all(conn, buffer);
	if(status == STATUS_SUCCESS)
	{
		//Obtiene el header
		buffer_unpack(buffer, &header.opcode, 1);
		buffer_unpack(buffer, &header.msgtype, 1);

		//Obtiene el payload
		void* msg = buffer_unpack_msg(header, buffer, conn);

		//Se invoca a la función 'closure' que recibe el mensaje
		closure(conn, header, msg);
	}

	return status;
}

static t_consulta* 		_buffer_to_consulta (int8_t msg_type, t_buffer* buffer);
static t_handshake_cli* _buffer_to_handshake_cli(t_buffer* buffer);
static t_handshake_res* _buffer_to_handshake_res(t_buffer* buffer, t_sfd conn);

static t_rta_handshake_cli*	_buffer_to_rta_handshake_cli(t_buffer* buffer);
static t_rta_cons_rest* _buffer_to_rta_cons_rest(t_buffer* buffer);
static t_rta_obt_rest*  _buffer_to_rta_obt_rest (t_buffer* buffer);
static t_rta_cons_pl*   _buffer_to_rta_cons_pl  (t_buffer* buffer);
static t_rta_crear_ped* _buffer_to_rta_crear_ped(t_buffer* buffer);
static t_rta_cons_ped*  _buffer_to_rta_cons_ped (t_buffer* buffer);
static t_rta_obt_ped*   _buffer_to_rta_obt_ped  (t_buffer* buffer);
static t_rta_obt_rec*   _buffer_to_rta_obt_rec  (t_buffer* buffer);

t_list* buffer_unpack_menu(t_buffer* buffer);
t_list* buffer_unpack_platos(t_buffer* buffer);
t_list* buffer_unpack_receta(t_buffer* buffer);

void* buffer_unpack_msg(t_header header, t_buffer* buffer, t_sfd conn)
{
	switch(header.opcode)
	{
	case OPCODE_CONSULTA:
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE:
			return (void*)_buffer_to_handshake_cli(buffer);
		case HANDSHAKE_RESTAURANTE:
			return (void*)_buffer_to_handshake_res(buffer, conn);
		default:
			return (void*)_buffer_to_consulta(header.msgtype, buffer);
		}
		break;
	case OPCODE_RESPUESTA_OK:
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE:
			return (void*)_buffer_to_rta_handshake_cli(buffer);
		case CONSULTAR_RESTAURANTES:
			return (void*)_buffer_to_rta_cons_rest(buffer);
		case OBTENER_RESTAURANTE:
			return (void*)_buffer_to_rta_obt_rest(buffer);
		case CONSULTAR_PLATOS:
			return (void*)_buffer_to_rta_cons_pl(buffer);
		case CREAR_PEDIDO:
			return (void*)_buffer_to_rta_crear_ped(buffer);
		case CONSULTAR_PEDIDO:
			return (void*)_buffer_to_rta_cons_ped(buffer);
		case OBTENER_PEDIDO:
			return (void*)_buffer_to_rta_obt_ped(buffer);
		case OBTENER_RECETA:
			return (void*)_buffer_to_rta_obt_rec(buffer);
		default:
			return NULL;
		}
		break;
	default:
		return NULL;
	}
}

static t_consulta* _buffer_to_consulta(int8_t msg_type, t_buffer* buffer)
{
	t_consulta* msg = malloc(sizeof(t_consulta));
	msg->msgtype = msg_type;

	int8_t self_module = (int8_t)cs_string_to_enum(cs_config_get_string("MODULO"), cs_enum_module_to_str);

	//Comida
	if(cs_cons_has_argument(msg_type, CONS_ARG_COMIDA, self_module)) {
		msg->comida = buffer_unpack_string(buffer);
	} else {
		msg->comida = NULL;
	}

	//Cantidad
	if(cs_cons_has_argument(msg_type, CONS_ARG_CANTIDAD, self_module)) {
		buffer_unpack(buffer, &msg->cantidad, sizeof(uint32_t));
	} else {
		msg->cantidad = 0;
	}

	//Restaurante
	if(cs_cons_has_argument(msg_type, CONS_ARG_RESTAURANTE, self_module)) {
		msg->restaurante = buffer_unpack_string(buffer);
	} else {
		msg->restaurante = NULL;
	}

	//Pedido id
	if(cs_cons_has_argument(msg_type, CONS_ARG_PEDIDO_ID, self_module)) {
		buffer_unpack(buffer, &msg->pedido_id, sizeof(uint32_t));
	} else {
		msg->pedido_id = 0;
	}

	return msg;
}

static t_handshake_cli* _buffer_to_handshake_cli(t_buffer* buffer)
{
	t_handshake_cli* msg = malloc(sizeof(t_handshake_cli));

	msg->nombre = buffer_unpack_string(buffer);                //Nombre
	buffer_unpack(buffer, &msg->posicion.x, sizeof(uint32_t)); //Posicion en x
	buffer_unpack(buffer, &msg->posicion.y, sizeof(uint32_t)); //Posicion en y

	return msg;
}

static t_handshake_res* _buffer_to_handshake_res(t_buffer* buffer, t_sfd conn)
{
	t_handshake_res* msg = malloc(sizeof(t_handshake_res));

	msg->nombre = buffer_unpack_string(buffer);                //Nombre
	buffer_unpack(buffer, &msg->posicion.x, sizeof(uint32_t)); //Posicion en x
	buffer_unpack(buffer, &msg->posicion.y, sizeof(uint32_t)); //Posicion en y
	msg->ip = NULL;
	cs_get_peer_info(conn, &msg->ip, NULL);                    //IP
	msg->puerto = buffer_unpack_string(buffer);                //Puerto

	return msg;
}

static t_rta_cons_rest* _buffer_to_rta_cons_rest(t_buffer* buffer)
{
	t_rta_cons_rest* msg = malloc(sizeof(t_rta_cons_rest));

	msg->restaurantes = buffer_unpack_string_array(buffer); //Restaurantes

	return msg;
}

static t_rta_obt_rest*  _buffer_to_rta_obt_rest(t_buffer* buffer)
{
	t_rta_obt_rest* msg = malloc(sizeof(t_rta_obt_rest));

	buffer_unpack(buffer, &msg->cant_cocineros   , sizeof(uint32_t)); //Cantidad de cocineros
	msg->afinidades = buffer_unpack_string_array(buffer);             //Afinidades
	msg->menu = buffer_unpack_menu(buffer);                           //Menú
	buffer_unpack(buffer, &msg->pos_restaurante.x, sizeof(uint32_t)); //Posición del restaurante en x
	buffer_unpack(buffer, &msg->pos_restaurante.y, sizeof(uint32_t)); //Posición del restaurante en y
	buffer_unpack(buffer, &msg->cant_hornos      , sizeof(uint32_t)); //Cantidad de hornos
	buffer_unpack(buffer, &msg->cant_pedidos     , sizeof(uint32_t)); //Cantidad de pedidos

	return msg;
}

static t_rta_cons_pl*   _buffer_to_rta_cons_pl(t_buffer* buffer)
{
	t_rta_cons_pl* msg = malloc(sizeof(t_rta_cons_pl));

	msg->comidas = buffer_unpack_string_array(buffer); //Platos

	return msg;
}

static t_rta_crear_ped* _buffer_to_rta_crear_ped(t_buffer* buffer)
{
	t_rta_crear_ped* msg = malloc(sizeof(t_rta_crear_ped));

	buffer_unpack(buffer, &msg->pedido_id, sizeof(uint32_t)); //Pedido ID

	return msg;
}

static t_rta_cons_ped*  _buffer_to_rta_cons_ped(t_buffer* buffer)
{
	t_rta_cons_ped* msg = malloc(sizeof(t_rta_cons_ped));

	msg->restaurante = buffer_unpack_string(buffer);             //Restaurante
	buffer_unpack(buffer, &msg->estado_pedido, sizeof(uint8_t)); //Estado del pedido
	msg->platos_y_estados = buffer_unpack_platos(buffer);                                //Platos

	return msg;
}

static t_rta_obt_ped*   _buffer_to_rta_obt_ped(t_buffer* buffer)
{
	t_rta_obt_ped* msg = malloc(sizeof(t_rta_obt_ped));

	buffer_unpack(buffer, &msg->estado_pedido, sizeof(uint8_t)); //Estado del pedido
	msg->platos_y_estados = buffer_unpack_platos(buffer);                                //Platos

	return msg;
}

static t_rta_obt_rec*   _buffer_to_rta_obt_rec(t_buffer* buffer)
{
	t_rta_obt_rec* msg = malloc(sizeof(t_rta_obt_rec));

	msg->pasos_receta = buffer_unpack_receta(buffer); //Receta

	return msg;
}

static t_rta_handshake_cli*	_buffer_to_rta_handshake_cli(t_buffer* buffer)
{
	t_rta_handshake_cli* msg = malloc(sizeof(t_rta_handshake_cli));

	buffer_unpack(buffer, &msg->modulo, sizeof(int8_t)); //Módulo

	return msg;
}

t_list* buffer_unpack_menu(t_buffer* buffer)
{
	t_comida_menu* _unpack_comida_menu(t_buffer* buffer) {
		t_comida_menu* comida_menu = malloc(sizeof(t_comida_menu));
		comida_menu->comida = buffer_unpack_string(buffer);
		buffer_unpack(buffer, &comida_menu->precio, sizeof(uint32_t));

		return comida_menu;
	}
	return buffer_unpack_list(buffer, (void*)_unpack_comida_menu);
}

t_list* buffer_unpack_platos(t_buffer* buffer)
{
	t_plato* _unpack_plato(t_buffer* buffer) {
		t_plato* plato = malloc(sizeof(t_plato));
		plato->comida = buffer_unpack_string(buffer);                //Comida
		buffer_unpack(buffer, &plato->cant_lista, sizeof(uint32_t)); //Cantidad lista
		buffer_unpack(buffer, &plato->cant_total, sizeof(uint32_t)); //Cantidad total

		return plato;
	}
	return buffer_unpack_list(buffer, (void*)_unpack_plato);
}

t_list* buffer_unpack_receta(t_buffer* buffer)
{
	t_paso_receta* _unpack_paso_receta(t_buffer* buffer) {
		t_paso_receta* paso_receta = malloc(sizeof(t_paso_receta));
		paso_receta->paso = buffer_unpack_string(buffer);
		buffer_unpack(buffer, &paso_receta->tiempo, sizeof(uint32_t));

		return paso_receta;
	}
	return buffer_unpack_list(buffer, (void*)_unpack_paso_receta);
}
