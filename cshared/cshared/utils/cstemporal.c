#include "cstemporal.h"

void  cs_temporal_do(void(*closure)(char*, char*))
{
	char* date_str = cs_temporal_get_string_date();
	char* time_str = cs_temporal_get_string_time();

	closure(date_str, time_str);

	free(date_str);
	free(time_str);
}

char* cs_temporal_get_string_time(void)
{
	time_t tiempo     = time(NULL);
	struct tm *log_tm = localtime(&tiempo);
	char* time_str    = string_duplicate("HH:MM:SS");

	strftime(time_str, string_length(time_str)+1, "%H:%M:%S", log_tm);

	return time_str;
}

char* cs_temporal_get_string_date(void)
{
	time_t tiempo     = time(NULL);
	struct tm *tlocal = localtime(&tiempo);
	char* date_str    = string_duplicate("dd/mm/yy");

	strftime(date_str, string_length(date_str)+1, "%d/%m/%y", tlocal);

	return date_str;
}
