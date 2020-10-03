#ifndef UTILS_TEMPORAL_H_
#define UTILS_TEMPORAL_H_

#include "cscore.h"
#include <string.h>
#include <time.h>
#include <sys/timeb.h>

	/**
	* @NAME: cs_temporal_get_string_time
	* @DESC: Retorna un string con la hora actual,
	* con el formato recibido por parámetro.
	* Ejemplos:
	* temporal_get_string_time("%d/%m/%y") => "30/09/20"
	* temporal_get_string_time("%H:%M:%S:%MS") => "12:51:59:331"
	* temporal_get_string_time("%d/%m/%y %H:%M:%S") => "30/09/20 12:51:59"
	*/
	char* cs_temporal_get_string_time(const char* format);

#endif
