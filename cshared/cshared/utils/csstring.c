#include "csstring.h"

static void _string_array_push(char*** array, char* text, int size);
static bool _string_is_in_num_array_format(char* str, bool _signed);

void cs_stream_copy(void* stream, int* offset_ptr, void* value, uint32_t value_size, bool buffer_is_dest)
{
	if(buffer_is_dest)
	{
		//Si el buffer es destino, se copia el contenido del 'value' al 'buffer'
		memcpy(stream + *offset_ptr, value, value_size);
		*offset_ptr += value_size;
	}
	else
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
	}
	else if(str[0] == '+' && str[1])
	{
		*number += cs_string_to_uint(str + 1);
		if(*number < 0) return -1;
	}
	else
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

bool cs_string_is_in_string_array_format(char* str)
{
	return string_starts_with(str, "[") && string_ends_with(str, "]") &&
		   !(string_contains(str, "[,") || string_contains(str, ",,") || string_contains(str, ",]"));
}

char* cs_string_array_to_string(char** str_arr)
{
	char* str = string_duplicate("[");

	void _append_lines_to_str(char* line)
	{
		string_append(&str, line);
		string_append(&str, ",");
	}
	string_iterate_lines(str_arr, _append_lines_to_str);
	str[strlen(str) - 1] = ']';

	return str;
}

char* cs_int_array_to_string(int* int_arr, int size)
{
	char* str = string_duplicate("[");

	for(int i = 0; i < size; i++)
	{
		string_append_with_format(&str, "%d,", int_arr[i]);
	}
	str[strlen(str) - 1] = ']';

	return str;
}

bool cs_string_is_in_uint_array_format(char* str)
{
	return _string_is_in_num_array_format(str, false);
}

bool cs_string_is_in_int_array_format(char* str)
{
	return _string_is_in_num_array_format(str, true);
}

char** string_array_new() {
	char** array = malloc(sizeof(char*));
	array[0] = NULL;

	return array;
}

int	string_array_size(char** str_arr) {
	int lines = 0;
	if(!str_arr) return -1;

	void _accumulate_lines(char* element) {
		lines++;
	}
	string_iterate_lines(str_arr,_accumulate_lines);

	return lines;
}

bool string_array_is_empty(char** array) {
	return array[0] == NULL;
}

void string_array_push(char*** array, char* text) {
	_string_array_push(array, text, string_array_size(*array));
}

static void _string_array_push(char*** array, char* text, int size) {
	*array = realloc(*array, sizeof(char*) * (size + 2));
	(*array)[size] = text;
	(*array)[size + 1] = NULL;
}

static bool _string_is_in_num_array_format(char* str, bool _signed)
{
	char** str_array;
	bool result = false;

	if(cs_string_is_in_string_array_format(str))
	{
		result = true;
		str_array = string_get_string_as_array(str);
		void _check_numbers(char* line)
		{
			int num;
			if(_signed) {
				if( cs_string_to_int(&num, line) < 0 ) result = false;
			} else {
				if( cs_string_to_uint(line) < 0 ) result = false;
			}
		}
		string_iterate_lines(str_array, _check_numbers);
		string_iterate_lines(str_array, (void*) free);
		free(str_array);
	}

	return result;
}
