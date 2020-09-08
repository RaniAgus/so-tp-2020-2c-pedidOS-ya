#include "csmsg.h"

static const char* CS_MSGTYPE_STR[] =
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
	return CS_MSGTYPE_STR[value];
}

/***********ELEMENTOS DE CADA TIPO DE SOLICITUD*********/

static const int MENSAJE_HAS_ELEMENT[CANT_MSGTYPES][MSG_ELEMENTS_CANT] =
{
           /*{plato, cant, rest, p_id}*/
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

static bool _mensaje_has_element(e_msgtype msgtype, e_msg_elem element)
{
	return MENSAJE_HAS_ELEMENT[(int)msgtype][(int)element];
}

/******************ELEMENT DESTROYERS**********************/

void cs_msg_destroy(t_mensaje* msg)
{
	free(msg->plato);
	free(msg->restaurante);
	free(msg);
}

void cs_rta_destroy(void* msg, e_msgtype msg_type)
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
    	//TODO: Recetas?
    	break;
    case CONSULTAR_PLATOS:
    	string_iterate_lines(RTA_CONS_PL(msg)->platos, (void*) free);
    	free(RTA_CONS_PL(msg)->platos);
        break;
    case CONSULTAR_PEDIDO:
    	free(RTA_CONS_PED(msg)->restaurante);
    	cs_platos_destroy(RTA_CONS_PED(msg)->platos_y_estados);
        break;
    case OBTENER_PEDIDO:
    	cs_platos_destroy(RTA_OBT_PED(msg)->platos_y_estados);
        break;
    case OBTENER_RECETA:
    	cs_receta_destroy(RTA_OBT_REC(msg)->pasos_receta);
        break;
    default:
    	break;
    }

	if(msg) free(msg);
}

void cs_receta_destroy(t_list* receta)
{
	void _element_destroyer(t_paso_receta* paso_receta)
	{
		free(paso_receta->paso);
		free(paso_receta);
	}
	list_destroy_and_destroy_elements(receta, (void*)_element_destroyer);
}

void cs_platos_destroy(t_list* platos)
{
	void _element_destroyer(t_plato_y_estado* plato)
	{
		free(plato->plato);
		free(plato);
	}
	list_destroy_and_destroy_elements(platos, (void*)_element_destroyer);
}

/***********************CREATE************************/

t_mensaje* 	cs_msg_create(e_msgtype msgtype,
						  char*     plato,
						  uint32_t  cant,
						  char*     rest,
						  uint32_t  pedido_id)
{
	t_mensaje* msg;

	CHECK_STATUS(MALLOC(msg, sizeof(t_mensaje)));

	msg->plato       = string_duplicate(plato);
	msg->cantidad    = cant;
	msg->restaurante = string_duplicate(rest);
	msg->pedido_id   = pedido_id;

	return msg;
}

/**********************TO STRING**********************/

char* cs_msg_to_str(t_mensaje* msg)
{
	char* mensaje_str = string_duplicate(
			(char*)cs_enum_msgtype_to_str((int)msg->msgtype)
	);

	if(_mensaje_has_element(msg->msgtype, MSG_PLATO))
	{
		string_append_with_format(
				&mensaje_str,
				" [PLATO: %s]",
				msg->plato
		);
	}
	if(_mensaje_has_element(msg->msgtype, MSG_CANTIDAD))
	{
		string_append_with_format(
				&mensaje_str,
				" [CANT: %d]",
				msg->cantidad
		);
	}
	if(_mensaje_has_element(msg->msgtype, MSG_RESTAURANTE))
	{
		string_append_with_format(
				&mensaje_str,
				" [RESTAURANTE: %s]",
				msg->restaurante
		);
	}
	if(_mensaje_has_element(msg->msgtype, MSG_PEDIDO_ID))
	{
		string_append_with_format(
				&mensaje_str,
				" [ID_PEDIDO: %d]",
				msg->pedido_id
		);
	}

	return mensaje_str;
}

//TODO: [MSG]: Respuesta a String
char* cs_rta_to_str(void* msg, e_msgtype msg_type)
{
	char* rta_str = string_duplicate(
			(char*)cs_enum_msgtype_to_str((int)msg_type)
	);

	switch(msg_type)
	{
	case CONSULTAR_RESTAURANTES:

	    break;
	case OBTENER_RESTAURANTE:

	   	break;
	case CONSULTAR_PLATOS:

	    break;
	case CONSULTAR_PEDIDO:

	    break;
	case OBTENER_PEDIDO:

	    break;
	case OBTENER_RECETA:

	    break;
	default:
	  	break;
	}

	return rta_str;
}
