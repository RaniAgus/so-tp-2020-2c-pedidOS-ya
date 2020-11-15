#ifndef RESTCORE_H_
#define RESTCORE_H_

#include <cshared/cshared.h>

char* mi_nombre;
t_pos mi_posicion;

typedef enum { ESTADO_NEW = 1, ESTADO_READY, ESTADO_BLOCK, ESTADO_EXEC, ESTADO_EXIT } rest_estado_e;

const char* rest_estado_to_str(rest_estado_e value);

#endif /* RESTCORE_H_ */
