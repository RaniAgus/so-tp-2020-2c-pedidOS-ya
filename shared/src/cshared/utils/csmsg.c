#include "csmsg.h"

//TODO: [MSG]: Tipo de mensaje a string
static const char* CS_MSGTYPE_STR[] =
{
	"UNKNOWN",
	"NEW_POKEMON",
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
    case NEW_POKEMON:
        free(MSG_NEW(msg)->name);
        free(msg);
        break;
    default:
        break;
    }
}

//TODO: [MSG]: Crear mensaje
t_msg_new* cs_create_msg_new(const char* pokemon, uint32_t posx, uint32_t posy, uint32_t cantidad)
{
	t_msg_new* msg;
	CHECK_STATUS(MALLOC(msg, sizeof(t_msg_new)));

    msg->name  = string_duplicate((char*)pokemon);
    msg->pos.x = posx;
    msg->pos.y = posy;
    msg->cant  = cantidad;

    return msg;
}

//TODO: [MSG]: Mensaje a string
static char* cs_msg_new_to_str(t_msg_new* msg);

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

static char* cs_msg_new_to_str(t_msg_new* msg)
{
	char* msg_str = string_new();

	string_append_with_format(
		&msg_str,
		"[POKEMON: %s] "
		"[X_POS: %d] "
		"[Y_POS: %d] "
		"[CANT: %d] ",
		msg->name,
		msg->pos.x,
		msg->pos.y,
		msg->cant
	);

	return msg_str;
}
