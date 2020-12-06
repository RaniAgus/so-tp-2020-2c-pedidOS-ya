#include "cssend.h"
#include "utils/cslog.h"

static e_status _send_msg(t_sfd conn, t_header header, void* msg, e_module dest);

e_status cs_send_handshake_cli(t_sfd conn)
{
	e_status status;
	t_header header = { OPCODE_CONSULTA, HANDSHAKE_CLIENTE };
	t_handshake_cli* msg = cs_cons_handshake_cli_create();

	status = _send_msg(conn, header, msg, MODULO_DESCONOCIDO);
	cs_msg_destroy(msg, header.opcode, header.msgtype);

	return status;
}

e_status cs_send_handshake_res(t_sfd conn, t_pos pos)
{
	e_status status;
	t_header header = { OPCODE_CONSULTA, HANDSHAKE_RESTAURANTE };
	t_handshake_res* msg = cs_cons_handshake_res_create(pos);

	status = _send_msg(conn, header, msg, MODULO_DESCONOCIDO);
	cs_msg_destroy(msg, header.opcode, header.msgtype);

	return status;
}

e_status cs_send_consulta(t_sfd conn, e_msgtype msg_type, t_consulta* msg, e_module dest)
{
	t_header header = { OPCODE_CONSULTA, msg_type };
	return _send_msg(conn, header, msg, dest);
}

e_status cs_send_respuesta(t_sfd conn, t_header header, void* msg)
{
	return _send_msg(conn, header, msg, MODULO_DESCONOCIDO);
}

static e_status _send_msg(t_sfd conn, t_header header, void* msg, e_module dest)
{
	e_status status;

	//Se agrega el header al paquete
	t_buffer* buffer = buffer_create();
	buffer_pack(buffer, &header.opcode, 1);
	buffer_pack(buffer, &header.msgtype, 1);

	//Se agrega el payload al paquete
	buffer_pack_msg(buffer, header, msg, dest);

	//Se envía al receptor
	status = cs_send_all(conn, buffer);

	//Se libera el buffer
	buffer_destroy(buffer);

	return status;
}

/*********************************  BUFFER  ***************************************/

void buffer_pack_consulta_hs_restaurante(t_buffer* buffer, t_handshake_res* msg);
void buffer_pack_consulta_hs_cliente(t_buffer* buffer, t_handshake_cli* msg);
void buffer_pack_consulta(t_buffer* buffer, t_consulta* msg, e_module dest);
void buffer_pack_respuesta_ok(t_buffer* buffer, e_msgtype msg_type, void* msg);

static void buffer_pack_handshake_cliente(t_buffer* buffer, t_rta_handshake_cli* msg);
static void buffer_pack_consultar_restaurantes(t_buffer* buffer, t_rta_cons_rest* msg);
static void buffer_pack_obtener_restaurante(t_buffer* buffer, t_rta_obt_rest* msg);
static void buffer_pack_consultar_platos(t_buffer* buffer, t_rta_cons_pl* msg);
static void buffer_pack_crear_pedido(t_buffer* buffer, t_rta_crear_ped* msg);
static void buffer_pack_consultar_pedido(t_buffer* buffer, t_rta_cons_ped* msg);
static void buffer_pack_obtener_pedido(t_buffer* buffer, t_rta_obt_ped* msg);
static void buffer_pack_obtener_receta(t_buffer* buffer, t_rta_obt_rec* msg);

void buffer_pack_menu(t_buffer* buffer, t_list* menu);
void buffer_pack_platos(t_buffer* buffer, t_list* platos);
void buffer_pack_receta(t_buffer* buffer, t_list* receta);

void buffer_pack_msg(t_buffer* buffer, t_header header, void* msg, e_module dest)
{
	switch(header.opcode)
	{
	case OPCODE_CONSULTA:
		switch(header.msgtype)
		{
		case HANDSHAKE_CLIENTE:
			buffer_pack_consulta_hs_cliente(buffer, msg);
			break;
		case HANDSHAKE_RESTAURANTE:
			buffer_pack_consulta_hs_restaurante(buffer, msg);
			break;
		default:
			buffer_pack_consulta(buffer, msg, dest);
			break;
		}
		break;
	case OPCODE_RESPUESTA_OK:
		buffer_pack_respuesta_ok(buffer, header.msgtype, msg);
		break;
	default:
		break;
	}
}

/********************************* CONSULTAS / HANDSHAKES  ***************************************/

void buffer_pack_consulta_hs_cliente(t_buffer* buffer, t_handshake_cli* msg)
{
	buffer_pack_string(buffer, msg->nombre);
	buffer_pack(buffer, &msg->posicion.x, sizeof(uint32_t));
	buffer_pack(buffer, &msg->posicion.y, sizeof(uint32_t));
}

void buffer_pack_consulta_hs_restaurante(t_buffer* buffer, t_handshake_res* msg)
{
	buffer_pack_string(buffer, msg->nombre);
	buffer_pack(buffer, &msg->posicion.x, sizeof(uint32_t));
	buffer_pack(buffer, &msg->posicion.y, sizeof(uint32_t));
	buffer_pack_string(buffer, msg->puerto);
}

void buffer_pack_consulta(t_buffer* buffer, t_consulta* msg, e_module dest)
{
	if(cs_cons_has_argument(msg->msgtype, CONS_ARG_COMIDA, dest))
		buffer_pack_string(buffer, msg->comida);

	if(cs_cons_has_argument(msg->msgtype, CONS_ARG_CANTIDAD, dest))
		buffer_pack(buffer, &msg->cantidad, sizeof(uint32_t));

	if(cs_cons_has_argument(msg->msgtype, CONS_ARG_RESTAURANTE, dest))
		buffer_pack_string(buffer, msg->restaurante);

	if(cs_cons_has_argument(msg->msgtype, CONS_ARG_PEDIDO_ID, dest))
		buffer_pack(buffer, &msg->pedido_id, sizeof(uint32_t));
}

/********************************* RESPUESTAS  ***************************************/

void buffer_pack_respuesta_ok(t_buffer* buffer, e_msgtype msg_type, void* msg)
{
	switch(msg_type)
	{
	case HANDSHAKE_CLIENTE:
		buffer_pack_handshake_cliente(buffer, msg);
		break;
	case CONSULTAR_RESTAURANTES:
		buffer_pack_consultar_restaurantes(buffer, msg);
		break;
	case OBTENER_RESTAURANTE:
		buffer_pack_obtener_restaurante(buffer, msg);
		break;
	case CONSULTAR_PLATOS:
		buffer_pack_consultar_platos(buffer, msg);
		break;
	case CREAR_PEDIDO:
		buffer_pack_crear_pedido(buffer, msg);
		break;
	case CONSULTAR_PEDIDO:
		buffer_pack_consultar_pedido(buffer, msg);
		break;
	case OBTENER_PEDIDO:
		buffer_pack_obtener_pedido(buffer, msg);
		break;
	case OBTENER_RECETA:
		buffer_pack_obtener_receta(buffer, msg);
		break;
	default:
		break;
	}
}

static void buffer_pack_handshake_cliente(t_buffer* buffer, t_rta_handshake_cli* msg)
{
	buffer_pack(buffer, &msg->modulo, sizeof(uint8_t)); //Modulo
}

static void buffer_pack_consultar_restaurantes(t_buffer* buffer, t_rta_cons_rest* msg)
{
	buffer_pack_string_array(buffer, msg->restaurantes); //Restaurantes
}

static void buffer_pack_obtener_restaurante(t_buffer* buffer, t_rta_obt_rest* msg)
{
	buffer_pack(buffer, &msg->cant_cocineros, sizeof(uint32_t));    //Cocineros (cantidad)
	buffer_pack_string_array(buffer, msg->afinidades);              //Cocineros (afinidades)
	buffer_pack_menu(buffer, msg->menu);                            //Menú (comidas + precios)
	buffer_pack(buffer, &msg->pos_restaurante.x, sizeof(uint32_t)); //Posición del restaurante en x
	buffer_pack(buffer, &msg->pos_restaurante.y, sizeof(uint32_t)); //Posición del restaurante en y
	buffer_pack(buffer, &msg->cant_hornos, sizeof(uint32_t));       //Cantidad de hornos
	buffer_pack(buffer, &msg->cant_pedidos, sizeof(uint32_t));      //Cantidad de pedidos
}

static void buffer_pack_consultar_platos(t_buffer* buffer, t_rta_cons_pl* msg)
{
	buffer_pack_string_array(buffer, msg->comidas); //Comidas
}


static void buffer_pack_crear_pedido(t_buffer* buffer, t_rta_crear_ped* msg)
{
	buffer_pack(buffer, &msg->pedido_id, sizeof(uint32_t)); //Pedido ID
}

static void buffer_pack_consultar_pedido(t_buffer* buffer, t_rta_cons_ped* msg)
{
	buffer_pack_string(buffer, msg->restaurante);            //Restaurante
	buffer_pack(buffer, &msg->estado_pedido,sizeof(int8_t)); //Estado del pedido
	buffer_pack_platos(buffer, msg->platos_y_estados);       //Platos (comidas + listos + totales)
}

static void buffer_pack_obtener_pedido(t_buffer* buffer, t_rta_obt_ped* msg)
{
	buffer_pack(buffer, &msg->estado_pedido,sizeof(int8_t));  //Estado del pedido
	buffer_pack_platos(buffer, msg->platos_y_estados);        //Platos (comidas + listos + totales)
}

static void buffer_pack_obtener_receta(t_buffer* buffer, t_rta_obt_rec* msg)
{
	buffer_pack_receta(buffer, msg->pasos_receta); //Receta (pasos + tiempos)
}

/********************************* LISTAS  ***************************************/

void buffer_pack_menu(t_buffer* buffer, t_list* menu)
{
	void _pack_comida_menu(t_buffer* buffer, t_comida_menu* comida_menu) {
		buffer_pack_string(buffer, comida_menu->comida);
		buffer_pack(buffer, &comida_menu->precio, sizeof(uint32_t));
	}
	buffer_pack_list(buffer, menu, (void*)_pack_comida_menu);
}

void buffer_pack_platos(t_buffer* buffer, t_list* platos)
{
	void _pack_plato(t_buffer* buffer, t_plato* plato) {
		buffer_pack_string(buffer, plato->comida);                 //Comida
		buffer_pack(buffer, &plato->cant_lista, sizeof(uint32_t)); //Cantidad lista
		buffer_pack(buffer, &plato->cant_total, sizeof(uint32_t)); //Cantidad total
	}
	buffer_pack_list(buffer, platos, (void*)_pack_plato);
}

void buffer_pack_receta(t_buffer* buffer, t_list* receta)
{
	void _pack_paso_receta(t_buffer* buffer, t_paso_receta* paso_receta) {
		buffer_pack_string(buffer, paso_receta->paso);
		buffer_pack(buffer, &paso_receta->tiempo, sizeof(uint32_t));
	}
	buffer_pack_list(buffer, receta, (void*)_pack_paso_receta);
}
