#ifndef UTILS_TEMPORAL_H_
#define UTILS_TEMPORAL_H_

#include "cscore.h"
#include <time.h>

/**
* @NAME cs_temporal_do
* @DESC Aplica la función 'closure', recibiendo por parámetro
* la fecha (dd/mm/yy) y la hora (HH:MM:SS) actual.
*/
void  cs_temporal_do(void(*closure)(char* date_str, char* time_str));

/**
* @NAME cs_temporal_get_string_time
* @DESC Retorna un string con la hora en formato hh:mm:ss
*/
char* cs_temporal_get_string_time(void);

/**
* @NAME cs_temporal_get_string_date
* @DESC Retorna un string con la fecha en formato DD/MM/AA
*/
char* cs_temporal_get_string_date(void);

#endif
