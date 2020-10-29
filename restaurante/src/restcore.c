#include "restcore.h"

static const char* _ESTADO_STR[] = {"Desconocido", "New", "Ready", "Block", "Exec", "Exit"};

const char* rest_estado_to_str(rest_estado_e value)
{
	return _ESTADO_STR[value];
}
