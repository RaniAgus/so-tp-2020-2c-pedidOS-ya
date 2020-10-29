#ifndef CSHARED_CSMSGTOSTRING_H_
#define CSHARED_CSMSGTOSTRING_H_

#include "csmsgtypes.h"

/**
* @NAME cs_enum_msgtype_to_str
* @DESC Devuelve el string correspondiente al enum value de e_msgtype
*/
const char*	cs_enum_msgtype_to_str(int value);

/**
* @NAME cs_msg_to_str
* @DESC Recibe un mensaje y devuelve un string con su contenido
* (normalmente para luego mostrarlo por pantalla en un log).
*/
char* 	cs_msg_to_str(void* msg, int8_t op_code, int8_t msg_type);

#endif /* CSHARED_CSMSGTOSTRING_H_ */
