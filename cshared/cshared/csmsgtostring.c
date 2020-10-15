#include "csmsgtostring.h"

static const char* _MSGTYPE_STR[] =
{
	"UNKNOWN",
	"CONSULTAR_RESTAURANTES",
	"SELECCIONAR_RESTAURANTE",
	"OBTENER_RESTAURANTE",
	"CONSULTAR_PLATOS",
	"CREAR_PEDIDO",
	"GUARDAR_PEDIDO",
	"AÑADIR_PLATO",
	"GUARDAR_PLATO",
	"CONFIRMAR_PEDIDO",
	"PLATO_LISTO",
	"CONSULTAR_PEDIDO",
	"OBTENER_PEDIDO",
	"FINALIZAR_PEDIDO",
	"TERMINAR_PEDIDO",
	"OBTENER_RECETA",
	"HANDSHAKE_CLIENTE",
	"HANDSHAKE_RESTAURANTE",
	NULL
};

const char* cs_enum_msgtype_to_str(int value)
{
	return _MSGTYPE_STR[value];
}

static const char* _MODULES_STR[] = {
		"Desconocido",
		"Comanda",
		"Sindicato",
		"Cliente",
		"App",
		"Restaurante",
		NULL
};

const char* cs_enum_module_to_str(int value) {
	return _MODULES_STR[value];
}

static void _cons_append(char** msg_str, t_consulta* msg);
static void _hs_cli_append(char** msg_str, t_handshake_cli* msg);
static void _hs_res_append(char** msg_str, t_handshake_res* msg);

static void _rta_handshake_cli_append(char** msg_str, t_rta_handshake_cli* msg);
static void _rta_cons_rest_append(char** msg_str, t_rta_cons_rest* msg);
static void _rta_obt_rest_append(char** msg_str, t_rta_obt_rest* msg);
static void _rta_cons_pl_append(char** msg_str, t_rta_cons_pl* msg);
static void _rta_crear_ped_append(char** msg_str, t_rta_crear_ped* msg);
static void _rta_cons_ped_append(char** msg_str, t_rta_cons_ped* msg);
static void _rta_obt_ped_append(char** msg_str, t_rta_obt_ped* msg);
static void _rta_obt_rec_append(char** msg_str, t_rta_obt_rec* msg);

char* cs_msg_to_str(void* msg, int8_t op_code, int8_t msg_type)
{
	char* msg_str = string_duplicate(
			(char*)cs_enum_msgtype_to_str((int)msg_type)
	);

	switch(op_code)
	{
	case OPCODE_CONSULTA:
		switch(msg_type)
		{
		case HANDSHAKE_CLIENTE:
			_hs_cli_append(&msg_str, (t_handshake_cli*)msg);
			break;
		case HANDSHAKE_RESTAURANTE:
			_hs_res_append(&msg_str, (t_handshake_res*)msg);
			break;
		default:
			_cons_append(&msg_str, (t_consulta*)msg);
			break;
		}
		break;
	case OPCODE_RESPUESTA_FAIL:
		string_append(&msg_str, " {RESULTADO: FAIL}");
		break;
	case OPCODE_RESPUESTA_OK:
		switch(msg_type)
		{
		case HANDSHAKE_CLIENTE:
			_rta_handshake_cli_append(&msg_str, (t_rta_handshake_cli*)msg);
			break;
		case CONSULTAR_RESTAURANTES:
			_rta_cons_rest_append(&msg_str, (t_rta_cons_rest*)msg);
		    break;
		case OBTENER_RESTAURANTE:
			_rta_obt_rest_append(&msg_str, (t_rta_obt_rest*)msg);
		   	break;
		case CONSULTAR_PLATOS:
			_rta_cons_pl_append(&msg_str, (t_rta_cons_pl*)msg);
		    break;
		case CREAR_PEDIDO:
			_rta_crear_ped_append(&msg_str, (t_rta_crear_ped*)msg);
			break;
		case CONSULTAR_PEDIDO:
			_rta_cons_ped_append(&msg_str, (t_rta_cons_ped*)msg);
		    break;
		case OBTENER_PEDIDO:
			_rta_obt_ped_append(&msg_str, (t_rta_obt_ped*)msg);
		    break;
		case OBTENER_RECETA:
			_rta_obt_rec_append(&msg_str, (t_rta_obt_rec*)msg);
		    break;
		default:
			string_append(&msg_str, " {RESULTADO: OK}");
		  	break;
		}
		break;
	default:
		break;
	}
	return msg_str;
}

// Private functions

static void _cons_append(char** msg_str, t_consulta* msg)
{
	if(msg->comida != NULL)
	{
		string_append_with_format(
				msg_str,
				" {COMIDA: %s}",
				msg->comida
		);
	}
	if(msg->cantidad)
	{
		string_append_with_format(
				msg_str,
				" {CANT: %d}",
				msg->cantidad
		);
	}
	if(msg->restaurante != NULL)
	{
		string_append_with_format(
				msg_str,
				" {RESTAURANTE: %s}",
				msg->restaurante
		);
	}
	if(msg->pedido_id)
	{
		string_append_with_format(
				msg_str,
				" {ID_PEDIDO: %d}",
				msg->pedido_id
		);
	}
}

static void _hs_cli_append(char** msg_str, t_handshake_cli* msg)
{
	string_append_with_format(
			msg_str,
			" {NOMBRE: %s} {POSX: %d} {POSY: %d}",
			msg->nombre,
			msg->posicion.x,
			msg->posicion.y
	);
}

static void _hs_res_append(char** msg_str, t_handshake_res* msg)
{
	string_append_with_format(
			msg_str,
			" {NOMBRE: %s} {POSX: %d} {POSY: %d}",
			msg->nombre,
			msg->posicion.x,
			msg->posicion.y
	);

	if(msg->ip)	string_append_with_format(msg_str, " {IP_ESCUCHA: %s}", msg->ip);
	string_append_with_format(msg_str, " {PUERTO_ESCUCHA: %s}", msg->puerto);
}

static void _rta_handshake_cli_append(char** msg_str, t_rta_handshake_cli* msg)
{
	string_append_with_format(
			msg_str,
			" {MODULO: %s}",
			cs_enum_module_to_str(msg->modulo)
	);
}

static void _rta_cons_rest_append(char** msg_str, t_rta_cons_rest* msg)
{
	string_append(msg_str, " {RESTAURANTES: [");

	void _restaurantes_append(char* restaurante)
	{
		string_append_with_format(msg_str, "%s,", restaurante);
	}
	string_iterate_lines(msg->restaurantes, _restaurantes_append);

	(*msg_str)[strlen(*msg_str)-1] = ']';
	string_append(msg_str, "}");
}

static void _rta_obt_rest_append(char** msg_str, t_rta_obt_rest* msg)
{
	string_append(msg_str, " {COCINEROS: [");

	for(int i=0; i < msg->cant_cocineros; i++)
	{
		string_append_with_format(msg_str, "(%d;%s),",
				i, ({ i < cs_string_array_lines_count(msg->afinidades)? msg->afinidades[i]:"Ninguna";}));
	}

	(*msg_str)[strlen(*msg_str)-1] = ']';
	string_append(msg_str, "}");

	string_append_with_format(msg_str, " {POSX: %d} {POSY: %d}",
			msg->pos_restaurante.x, msg->pos_restaurante.y);

	string_append(msg_str, " {MENÚ: [");

	void _pasos_receta_append(t_comida_menu* comida_menu)
	{
		string_append_with_format(msg_str, "%s($%d),",
				comida_menu->comida,
				comida_menu->precio);
	}
	list_iterate(msg->menu, (void*) _pasos_receta_append);

	(*msg_str)[strlen(*msg_str)-1] = ']';
	string_append(msg_str, "}");

	string_append_with_format(msg_str,
			" {CANT_HORNOS: %d} {CANT_PEDIDOS: %d}",
			msg->cant_hornos,
			msg->cant_pedidos
	);
}

static void _rta_cons_pl_append(char** msg_str, t_rta_cons_pl* msg)
{
	string_append(msg_str, " {PLATOS: [");

	void _platos_append(char* plato)
	{
		string_append_with_format(msg_str, "%s,", plato);
	}
	string_iterate_lines(msg->comidas, _platos_append);

	(*msg_str)[strlen(*msg_str)-1] = ']';
	string_append(msg_str, "}");
}

static void _rta_crear_ped_append(char** msg_str, t_rta_crear_ped* msg)
{
	string_append_with_format(msg_str, " {ID_PEDIDO: %d}",
			msg->pedido_id);
}

static void _platos_append(char** msg_str, t_list* platos_y_estados)
{
	string_append(msg_str, " {ESTADO_PLATOS: [");

	void _plato_y_estado_append(t_plato* plato_y_estado)
	{
		string_append_with_format(msg_str, "%s(%d/%d),",
				plato_y_estado->comida,
				plato_y_estado->cant_lista,
				plato_y_estado->cant_total);
	}
	list_iterate(platos_y_estados, (void*) _plato_y_estado_append);

	(*msg_str)[strlen(*msg_str)-1] = ']';
	string_append(msg_str, "}");
}

static void _rta_cons_ped_append(char** msg_str, t_rta_cons_ped* msg)
{
	string_append_with_format(msg_str, " {RESTAURANTE: %s} {ESTADO_PEDIDO: %s}",
			msg->restaurante, cs_enum_estado_pedido_to_str(msg->estado_pedido));

	_platos_append(msg_str, msg->platos_y_estados);
}

static void _rta_obt_ped_append(char** msg_str, t_rta_obt_ped* msg)
{
	string_append_with_format(msg_str, " {ESTADO_PEDIDO: %s}",
			cs_enum_estado_pedido_to_str(msg->estado_pedido));

	_platos_append(msg_str, msg->platos_y_estados);
}

static void _rta_obt_rec_append(char** msg_str, t_rta_obt_rec* msg)
{
	string_append(msg_str, " {PASOS_RECETA: [");

	void _pasos_receta_append(t_paso_receta* plato_y_estado)
	{
		string_append_with_format(msg_str, "(%s;%d),",
				plato_y_estado->paso,
				plato_y_estado->tiempo);
	}
	list_iterate(msg->pasos_receta, (void*) _pasos_receta_append);

	(*msg_str)[strlen(*msg_str)-1] = ']';
	string_append(msg_str, "}");
}
