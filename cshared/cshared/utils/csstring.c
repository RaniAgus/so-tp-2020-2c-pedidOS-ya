#include "csstring.h"

static int _string_to_int_array(int** int_arr, char* str, bool is_unsigned);

void cs_stream_copy(void* stream, int* offset_ptr, void* value, uint32_t value_size,
					int buffer_is_dest)
{
	if(buffer_is_dest)
	{
		//Si el buffer es destino, se copia el contenido del 'value' al 'buffer'
		memcpy(stream + *offset_ptr, value, value_size);
		*offset_ptr += value_size;
	}else
	{
		//Sino, se copia el contenido del 'buffer' al 'value'
		memcpy(value, stream + *offset_ptr, value_size);
		*offset_ptr += value_size;
	}
}

int cs_string_to_enum(const char* str, const char* (*enum_to_str)(int))
{
	int i = 0;
	while(enum_to_str(i) != NULL)
	{
		if(!strcmp(enum_to_str(i), str)) return i;
		i++;
	}

	return 0;
}

int cs_string_to_int(int* number, const char* str)
{
	*number = 0;

	if(!str) return -1;

	if(str[0] == '-' && str[1])
	{
		*number -= cs_string_to_uint(str + 1);
		if(*number > 0) return -1;
	} else
	{
		*number += cs_string_to_uint(str);
		if(*number < 0) return -1;
	}

	return 0;
}

int cs_string_to_uint(const char* str)
{
	for(const char* c = str; *c != '\0'; c++)
	{
		if(!isdigit(*c)) return -1;
	}

	return atoi(str);
}

int	cs_string_array_lines_count(char** str_arr)
{
	int lines = 0;
	if(!str_arr) return -1;

	void _accumulate_lines(char* element)
	{
		lines++;
	}
	string_iterate_lines(str_arr,_accumulate_lines);

	return lines;
}

char** cs_string_get_as_array(char* str)
{
	char** str_array = NULL;

	//Resuelve el problema de los corchetes
	if(string_starts_with(str, "[") && string_ends_with(str, "]"))
	{
		str_array = string_get_string_as_array(str);

		//TODO: cs_string_get_as_array - Revisar si cambia la firma de 'string_split'
		//Resuelve el problema de las comas (cambiar a revisar líneas vacías)
		int commas = 0;
		for(int i=0; str[i] != '\0'; i++)
			if(str[i]==',') commas++;

		int lines = cs_string_array_lines_count(str_array);
		if(commas >= lines)
		{
			string_iterate_lines(str_array, (void*) free);
			free(str_array);
			str_array = NULL;
		}
	}

	return str_array;
}

bool cs_string_is_array_format(char* str)
{
	char** str_array = cs_string_get_as_array(str);

	return (str_array == NULL)? false :
		({  string_iterate_lines(str_array, (void*) free); free(str_array); true; });
}

int cs_string_to_uint_array(int** int_arr, char* str)
{
	return _string_to_int_array(int_arr, str, true);
}

int cs_string_to_int_array(int** int_arr, char* str)
{
	return _string_to_int_array(int_arr, str, false);
}

//TODO: cs_string_to_enum_str_array - hacer publica
char** cs_string_to_enum_str_array(char* str, char* error_value)
{
	char* str2 = string_duplicate(str);
	str2[0] = ',';

	char* str_with_error_value = string_new();
	string_append_with_format(&str_with_error_value,
		"[%s%s", error_value, str
	);

	char** result = cs_string_get_as_array(str_with_error_value);

	//TODO: cs_string_to_enum_str_array - Quitar repetidos

	free(str2);
	free(str_with_error_value);

	return result;
}

static int _string_to_int_array(int** int_arr, char* str, bool is_unsigned)
{
	char** str_array;
	int lines;

	str_array = cs_string_get_as_array(str);
	if(!str_array) return -1;

	lines = cs_string_array_lines_count(str_array);

	if(int_arr) *int_arr = calloc(lines,sizeof(int));

	for(int i = 0; i < lines; i++)
	{
		int num, res;
		if(is_unsigned)
			res = num = cs_string_to_uint(str_array[i]);
		else
			res = cs_string_to_int(&num, str_array[i]);

		if(res < 0)
		{
			if(int_arr) free(*int_arr);
			lines = -1;
			break;
		}
		if(int_arr) (*int_arr)[i] = num;
	}
	string_iterate_lines(str_array, (void*) free);
	free(str_array);

	return lines;
}