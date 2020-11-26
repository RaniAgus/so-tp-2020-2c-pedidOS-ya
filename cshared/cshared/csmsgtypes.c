#include "csmsgtypes.h"

static void _rta_destroy(void* msg, int8_t msg_type);

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

static const int _MSG_ARGS[MSGTYPES_CANT][CONS_ARGS_CANT] =
{
/*           {comid, cant, rest, p_id}*/
/*UNKNOWN  */{  0  ,  0  ,  0  ,  0  },
/*CONS_RES */{  0  ,  0  ,  0  ,  0  },
/*SEL_RES  */{  0  ,  0  ,  1  ,  0  },
/*OBT_RES  */{  0  ,  0  ,  1  ,  0  },
/*CONS_PL  */{  0  ,  0  , -1  ,  0  },
/*CREAR_PED*/{  0  ,  0  ,  0  ,  0  },
/*GUARD_PED*/{  0  ,  0  ,  1  ,  1  },
/*AÑAD_PL  */{  1  ,  0  ,  0  ,  1  },
/*GUARD_PL */{  1  ,  1  ,  1  ,  1  },
/*CONF_PED */{  0  ,  0  , -1  ,  1  },
/*PL_LISTO */{  1  ,  0  ,  1  ,  1  },
/*CONS_PED */{  0  ,  0  ,  0  ,  1  },
/*OBT_PED  */{  0  ,  0  ,  1  ,  1  },
/*FIN_PED  */{  0  ,  0  ,  1  ,  1  },
/*TERM_PED */{  0  ,  0  ,  1  ,  1  },
/*OBT_REC  */{  1  ,  0  ,  0  ,  0  },
/*HNDSH_CLI*/{  0  ,  0  ,  0  ,  0  },
/*HNDSH_RES*/{  0  ,  0  ,  0  ,  0  }
};

bool cs_cons_has_argument(int8_t msgtype, int8_t arg, int8_t module)
{
	int result = _MSG_ARGS[(int)msgtype][(int)arg];

	if(result < 0) result = ({ module < 3 ? 1 : 0; });

	return result;
}


void cs_msg_destroy(void* msg, int8_t op_code, int8_t msg_type)
{
	switch(op_code)
	{
	case OPCODE_CONSULTA:
		switch (msg_type)
		{
		case HANDSHAKE_CLIENTE:
			free(HANDSHAKE_CLIENTE_PTR(msg)->nombre);
			break;
		case HANDSHAKE_RESTAURANTE:
			free(HANDSHAKE_RESTAURANTE_PTR(msg)->nombre);
			if(HANDSHAKE_RESTAURANTE_PTR(msg)->ip) free(HANDSHAKE_RESTAURANTE_PTR(msg)->ip);
			free(HANDSHAKE_RESTAURANTE_PTR(msg)->puerto);
			break;
		default:
			if(CONSULTA_PTR(msg)->comida)      free(CONSULTA_PTR(msg)->comida);
			if(CONSULTA_PTR(msg)->restaurante) free(CONSULTA_PTR(msg)->restaurante);
			break;
		}
		free(msg);
		break;
	case OPCODE_RESPUESTA_OK:
		_rta_destroy(msg, msg_type);
		break;
	default:
		break;
	}
}

t_consulta* 	_cons_create(int8_t msg_type, char* comida, uint32_t cant, char* rest, uint32_t pedido_id)
{
	t_consulta* msg;
	msg = malloc(sizeof(t_consulta));

	msg->msgtype = msg_type;

	msg->comida      = ({ comida != NULL ? string_duplicate(comida) : NULL; });
	msg->cantidad    = cant;
	msg->restaurante = ({ rest != NULL ?   string_duplicate(rest)   : NULL; });
	msg->pedido_id   = pedido_id;

	return msg;
}

t_handshake_cli* 	cs_cons_handshake_cli_create(void)
{
	t_handshake_cli* msg;
	msg = malloc(sizeof(t_handshake_cli));

	msg->nombre     = string_duplicate(cs_config_get_string("ID_CLIENTE"));
	msg->posicion.x = (uint32_t)cs_config_get_int("POSICION_X");
	msg->posicion.y = (uint32_t)cs_config_get_int("POSICION_Y");

	return msg;
}

t_handshake_res* cs_cons_handshake_res_create(t_pos pos)
{
	t_handshake_res* msg;
	msg = malloc(sizeof(t_handshake_res));

	msg->nombre     = string_duplicate(cs_config_get_string("NOMBRE_RESTAURANTE"));
	msg->posicion.x = pos.x;
	msg->posicion.y = pos.y;

	msg->ip     = NULL;
	msg->puerto = string_duplicate(cs_config_get_string("PUERTO_ESCUCHA"));

	return msg;
}

t_rta_handshake_cli* cs_rta_handshake_cli_create(void)
{
	t_rta_handshake_cli* rta;
	rta = malloc(sizeof(t_rta_handshake_cli));

	rta->modulo = (int8_t)cs_string_to_enum(cs_config_get_string("MODULO"), cs_enum_module_to_str);

	return rta;
}

t_rta_cons_rest* cs_rta_consultar_rest_create(char** restaurantes)
{
	t_rta_cons_rest* rta;
	rta = malloc(sizeof(t_rta_cons_rest));

	rta->restaurantes = restaurantes;

	return rta;
}

t_rta_obt_rest* cs_rta_obtener_rest_create(uint32_t cant_cocineros,
								   	   	   char* 	afinidades,
										   char*	comidas,
										   char*	precios,
										   t_pos 	pos_restaurante,
										   uint32_t cant_hornos,
										   uint32_t cant_pedidos)
{
	t_rta_obt_rest* rta;
	rta = malloc(sizeof(t_rta_obt_rest));

	rta->cant_cocineros    = cant_cocineros;
	rta->afinidades 	   = string_get_string_as_array(afinidades);
	rta->menu			   = cs_menu_create(comidas, precios);
	rta->pos_restaurante.x = pos_restaurante.x;
	rta->pos_restaurante.y = pos_restaurante.y;
	rta->cant_hornos 	   = cant_hornos;
	rta->cant_pedidos      = cant_pedidos;

	return rta;

}

t_rta_cons_pl* cs_rta_consultar_pl_create(char* platos)
{
	t_rta_cons_pl* rta;
	rta = malloc(sizeof(t_rta_cons_rest));

	rta->comidas = string_get_string_as_array(platos);

	return rta;
}

t_rta_crear_ped* cs_rta_crear_ped_create(uint32_t pedido_id)
{
	t_rta_crear_ped* rta;
	rta = malloc(sizeof(t_rta_crear_ped));

	rta->pedido_id = pedido_id;

	return rta;
}

t_rta_cons_ped* cs_rta_consultar_ped_create(char* rest, e_estado_ped estado_ped,
								   char* platos,
								   char* listos,
								   char* totales)
{
	t_rta_cons_ped* rta;
	rta = malloc(sizeof(t_rta_cons_ped));

	rta->restaurante      = string_duplicate(rest);
	rta->estado_pedido    = estado_ped;
	rta->platos_y_estados = cs_platos_create(platos, listos, totales);

	return rta;
}

t_rta_obt_ped* cs_rta_obtener_ped_create(e_estado_ped estado_ped, char* platos, char* listos, char* totales)
{
	t_rta_obt_ped* rta;
	rta = malloc(sizeof(t_rta_obt_ped));

	rta->estado_pedido    = estado_ped;
	rta->platos_y_estados = cs_platos_create(platos, listos, totales);

	return rta;
}

t_rta_obt_rec* cs_rta_obtener_receta_create(char* pasos, char* tiempos)
{
	t_rta_obt_rec* rta;
	rta = malloc(sizeof(t_rta_obt_rec));

	rta->pasos_receta = cs_receta_create(pasos, tiempos);

	return rta;
}

// Private functions

static void _rta_destroy(void* msg, int8_t msg_type)
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
    	string_iterate_lines(RTA_CONS_PL(msg)->comidas, (void*) free);
    	free(RTA_CONS_PL(msg)->comidas);
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
