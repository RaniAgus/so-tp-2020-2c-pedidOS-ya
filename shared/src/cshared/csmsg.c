#include "csmsg.h"

static void _msg_append(char** msg_str, t_mensaje* msg);

static void _rta_cons_rest_append(char** msg_str, t_rta_cons_rest* msg);
static void _rta_obt_rest_append(char** msg_str, t_rta_obt_rest* msg);
static void _rta_cons_pl_append(char** msg_str, t_rta_cons_pl* msg);
static void _rta_crear_ped_append(char** msg_str, t_rta_crear_ped* msg);
static void _rta_cons_ped_append(char** msg_str, t_rta_cons_ped* msg);
static void _rta_obt_ped_append(char** msg_str, t_rta_obt_ped* msg);
static void _rta_obt_rec_append(char** msg_str, t_rta_obt_rec* msg);

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
	NULL
};

const char* cs_enum_msgtype_to_str(int value)
{
	return _MSGTYPE_STR[value];
}

static void _rta_destroy(void* msg, e_msgtype msg_type);

void cs_msg_destroy(void* msg, e_opcode op_code, e_msgtype msg_type)
{
	switch(op_code)
	{
	case OPCODE_MENSAJE:
		free(MENSAJE_PTR(msg)->comida);
		free(MENSAJE_PTR(msg)->restaurante);
		free(msg);
		break;
	case OPCODE_RESPUESTA_OK:
		_rta_destroy(msg, msg_type);
		break;
	default:
		break;
	}
}

char* cs_msg_to_str(void* msg, e_opcode op_code, e_msgtype msg_type)
{
	char* msg_str = string_duplicate(
			(char*)cs_enum_msgtype_to_str((int)msg_type)
	);

	switch(op_code)
	{
	case OPCODE_MENSAJE:
		_msg_append(&msg_str, (t_mensaje*)msg);
		break;
	case OPCODE_RESPUESTA_FAIL:
		string_append(&msg_str, " {RESULTADO: FAIL}");
		break;
	case OPCODE_RESPUESTA_OK:
		switch(msg_type)
		{
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

t_mensaje* 	cs_msg_create(e_msgtype msgtype, char* plato, uint32_t cant, char* rest, uint32_t pedido_id)
{
	t_mensaje* msg;
	CHECK_STATUS(MALLOC(msg, sizeof(t_mensaje)));

	msg->msgtype = msgtype;

	msg->comida      = string_duplicate(plato);
	msg->cantidad    = cant;
	msg->restaurante = string_duplicate(rest);
	msg->pedido_id   = pedido_id;

	return msg;
}

t_rta_cons_rest* cs_rta_consultar_rest_create(char* restaurantes)
{
	t_rta_cons_rest* rta;
	CHECK_STATUS(MALLOC(rta, sizeof(t_rta_cons_rest)));

	rta->restaurantes = string_get_string_as_array(restaurantes);

	return rta;
}

t_rta_obt_rest* cs_rta_obtener_rest_create(uint32_t cant_cocineros,
								   	   	   char* 	afinidades,
										   char*	comidas,
										   char*	precios,
										   t_pos 	pos_restaurante,
										   uint32_t cant_hornos)
{
	t_rta_obt_rest* rta;
	CHECK_STATUS(MALLOC(rta, sizeof(t_rta_obt_rest)));

	rta->cant_cocineros    = cant_cocineros;
	rta->afinidades 	   = string_get_string_as_array(afinidades);
	rta->menu			   = cs_menu_create(comidas, precios);
	rta->pos_restaurante.x = pos_restaurante.x;
	rta->pos_restaurante.y = pos_restaurante.y;
	rta->cant_hornos 	   = cant_hornos;

	return rta;

}

t_rta_cons_pl* cs_rta_consultar_pl_create(char* platos)
{
	t_rta_cons_pl* rta;
	CHECK_STATUS(MALLOC(rta, sizeof(t_rta_cons_rest)));

	rta->platos = string_get_string_as_array(platos);

	return rta;
}

t_rta_crear_ped* cs_rta_crear_ped_create(uint32_t pedido_id)
{
	t_rta_crear_ped* rta;
	CHECK_STATUS(MALLOC(rta, sizeof(t_rta_crear_ped)));

	rta->pedido_id = pedido_id;

	return rta;
}

t_rta_cons_ped* cs_rta_consultar_ped_create(char* rest, e_estado_ped estado_ped,
								   char* platos,
								   char* listos,
								   char* totales)
{
	t_rta_cons_ped* rta;
	CHECK_STATUS(MALLOC(rta, sizeof(t_rta_cons_ped)));

	rta->restaurante      = string_duplicate(rest);
	rta->estado_pedido    = estado_ped;
	rta->platos_y_estados = cs_platos_create(platos, listos, totales);

	return rta;
}

t_rta_obt_ped* cs_rta_obtener_ped_create(char* platos, char* listos, char* totales)
{
	t_rta_obt_ped* rta;
	CHECK_STATUS(MALLOC(rta, sizeof(t_rta_obt_ped)));

	rta->platos_y_estados = cs_platos_create(platos, listos, totales);

	return rta;
}

t_rta_obt_rec* cs_rta_obtener_receta_create(char* pasos, char* tiempos)
{
	t_rta_obt_rec* rta;
	CHECK_STATUS(MALLOC(rta, sizeof(t_rta_obt_rec)));

	rta->pasos_receta = cs_receta_create(pasos, tiempos);

	return rta;
}

/**********************TO STRING**********************/

static void _msg_append(char** msg_str, t_mensaje* msg)
{

	if(cs_msg_has_argument(msg->msgtype, MSG_COMIDA))
	{
		string_append_with_format(
				msg_str,
				" {PLATO: %s}",
				msg->comida
		);
	}
	if(cs_msg_has_argument(msg->msgtype, MSG_CANTIDAD))
	{
		string_append_with_format(
				msg_str,
				" {CANT: %d}",
				msg->cantidad
		);
	}
	if(cs_msg_has_argument(msg->msgtype, MSG_RESTAURANTE))
	{
		string_append_with_format(
				msg_str,
				" {RESTAURANTE: %s}",
				msg->restaurante
		);
	}
	if(cs_msg_has_argument(msg->msgtype, MSG_PEDIDO_ID))
	{
		string_append_with_format(
				msg_str,
				" {ID_PEDIDO: %d}",
				msg->pedido_id
		);
	}
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
		string_append_with_format(msg_str, "%s ($%d),",
				comida_menu->comida,
				comida_menu->precio);
	}
	list_iterate(msg->menu, (void*) _pasos_receta_append);

	(*msg_str)[strlen(*msg_str)-1] = ']';
	string_append(msg_str, "}");

	string_append_with_format(msg_str, " {CANT_HORNOS: %d}",
			msg->cant_hornos);
}

static void _rta_cons_pl_append(char** msg_str, t_rta_cons_pl* msg)
{
	string_append(msg_str, " {PLATOS: [");

	void _platos_append(char* plato)
	{
		string_append_with_format(msg_str, "%s,", plato);
	}
	string_iterate_lines(msg->platos, _platos_append);

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
		string_append_with_format(msg_str, "%s (%d/%d),",
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
	_platos_append(msg_str, msg->platos_y_estados);
}

static void _rta_obt_rec_append(char** msg_str, t_rta_obt_rec* msg)
{
	string_append(msg_str, " {PASOS_RECETA: [");

	void _pasos_receta_append(t_paso_receta* plato_y_estado)
	{
		string_append_with_format(msg_str, "%s (%d),",
				plato_y_estado->paso,
				plato_y_estado->tiempo);
	}
	list_iterate(msg->pasos_receta, (void*) _pasos_receta_append);

	(*msg_str)[strlen(*msg_str)-1] = ']';
	string_append(msg_str, "}");
}

/******************ELEMENT DESTROYERS**********************/

static void _rta_destroy(void* msg, e_msgtype msg_type)
{
    switch(msg_type)
    {
    case CONSULTAR_RESTAURANTES:
    	string_iterate_lines(RTA_CONS_REST(msg)->restaurantes, (void*) free);
    	free(RTA_CONS_REST(msg)->restaurantes);
        break;
    case OBTENER_RESTAURANTE:
    	string_iterate_lines(RTA_OBT_REST(msg)->afinidades, (void*) free);
    	free(RTA_OBT_REST(msg)->afinidades);
    	cs_menu_destroy(RTA_OBT_REST(msg)->menu);
    	break;
    case CONSULTAR_PLATOS:
    	string_iterate_lines(RTA_CONS_PL(msg)->platos, (void*) free);
    	free(RTA_CONS_PL(msg)->platos);
        break;
    case CONSULTAR_PEDIDO:
    	free(RTA_CONSULTAR_PED(msg)->restaurante);
    	cs_platos_destroy(RTA_CONSULTAR_PED(msg)->platos_y_estados);
        break;
    case OBTENER_PEDIDO:
    	cs_platos_destroy(RTA_OBTENER_PED(msg)->platos_y_estados);
        break;
    case OBTENER_RECETA:
    	cs_receta_destroy(RTA_OBTENER_RECETA(msg)->pasos_receta);
        break;
    default:
    	break;
    }

	if(msg) free(msg);
}

/***********ELEMENTOS DE CADA TIPO DE SOLICITUD*********/

static const int _MSG_ARGS[MSGTYPES_CANT][MSG_ARGS_CANT] =
{
           /*{comida, cant, rest, p_id}*/
/*UNKNOWN  */{  0  ,  0  ,  0  ,  0  },
/*CONS_RES */{  0  ,  0  ,  0  ,  0  },
/*SEL_RES  */{  0  ,  0  ,  1  ,  0  }, //todo: ¿¿qué es el cliente??
/*OBT_RES  */{  0  ,  0  ,  1  ,  0  },
/*CONS_PL  */{  0  ,  0  ,  1  ,  0  },
/*CREAR_PED*/{  0  ,  0  ,  0  ,  0  },
/*GUARD_PED*/{  0  ,  0  ,  1  ,  1  },
/*AÑAD_PL  */{  1  ,  0  ,  0  ,  1  },
/*GUARD_PL */{  1  ,  1  ,  1  ,  1  },//todo: ¿¿para qué cantidad??
/*CONF_PED */{  0  ,  0  ,  0  ,  1  },
/*PL_LISTO */{  1  ,  0  ,  1  ,  1  },
/*CONS_PED */{  0  ,  0  ,  0  ,  1  },
/*OBT_PED  */{  0  ,  0  ,  1  ,  1  },
/*FIN_PED  */{  0  ,  0  ,  1  ,  1  },
/*TERM_PED */{  0  ,  0  ,  1  ,  1  },
/*OBT_REC  */{  1  ,  0  ,  0  ,  0  }

};

bool cs_msg_has_argument(e_msgtype msgtype, e_msg_arg arg)
{
	return _MSG_ARGS[(int)msgtype][(int)arg];
}
