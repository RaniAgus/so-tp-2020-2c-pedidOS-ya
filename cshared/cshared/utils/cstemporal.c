#include "cstemporal.h"

char* cs_temporal_get_string_time(const char* format) {
	char* str_time = malloc(strlen(format) + 1);
	char* strftime_format = strdup(format);
	struct tm *log_tm = malloc(sizeof(struct tm));

	//time
	time_t log_time = time(NULL);
	if(log_time == -1) {
		return NULL;
	}

	//miliseconds
	struct timeb tmili;
	if(ftime(&tmili)) {
		return NULL;
	}

	//parse
	char* aux = strftime_format;
	while((aux = strstr(aux, "%MS"))) {
		sprintf(aux, "%03hu", tmili.millitm);
		strftime_format[strlen(strftime_format)] = format[strlen(strftime_format)];
		aux = aux + 3;
	}

	localtime_r(&log_time, log_tm);
	strftime(str_time, strlen(format)+1, strftime_format, log_tm);

	free(log_tm);
	free(strftime_format);

	return str_time;
}
