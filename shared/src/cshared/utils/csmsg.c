#include "csmsg.h"

//TODO: [MSG]: Tipo de mensaje a string
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

//TODO: [MSG]: Destruir mensajes
void cs_msg_destroy(void* msg, e_msgtype msg_type)
{
    switch (msg_type)
    {
    case CONSULTAR_RESTAURANTES:
        break;
    case SELECCIONAR_RESTAURANTE:
        break;
    case OBTENER_RESTAURANTE:
        break;
    case CONSULTAR_PLATOS:
        break;
    case CREAR_PEDIDO:
        break;
    case GUARDAR_PEDIDO:
        break;
    case ANIADIR_PLATO:
        break;
    case GUARDAR_PLATO:
        break;
    case CONFIRMAR_PEDIDO:
        break;
    case PLATO_LISTO:
        break;
    case CONSULTAR_PEDIDO:
        break;
    case OBTENER_PEDIDO:
        break;
    case FINALIZAR_PEDIDO:
        break;
    case TERMINAR_PEDIDO:
        break;
    case OBTENER_RECETA:
        break;
    default:
        break;
    }
}

//TODO: [MSG]: Crear mensaje


//TODO: [MSG]: Mensaje a string

char* cs_msg_to_str(void* msg, e_msgtype msgtype)
{
	switch(msgtype)
	{
	case NEW_POKEMON:
		return cs_msg_new_to_str((t_msg_new*)msg);
	default:
		return NULL;
	}
}


