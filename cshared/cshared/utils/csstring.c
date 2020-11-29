#include "csstring.h"

static void _string_array_push(char*** array, char* text, int size);

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

int cs_string_to_uint(const char* str)
{
	for(const char* c = str; *c != '\0'; c++)
	{
		if(!isdigit(*c)) return -1;
	}

	return atoi(str);
}

bool cs_string_is_string_array(char* str)
{
	return string_starts_with(str, "[") && string_ends_with(str, "]") &&
		   !(string_contains(str, "[,") || string_contains(str, ",,") || string_contains(str, ",]"));
}

bool cs_string_is_unsigned_int_array(char* str)
{
	char** str_array;
	bool result = false;

	if(cs_string_is_string_array(str))
	{
		result = true;
		str_array = string_get_string_as_array(str);
		void _check_numbers(char* line)
		{
			if(cs_string_to_uint(line) < 0) result = false;
		}
		string_iterate_lines(str_array, _check_numbers);
		string_iterate_lines(str_array, (void*) free);
		free(str_array);
	}

	return result;
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

void string_n_append(char** original, char* string_to_add, int n) {
	if(strlen(string_to_add) < n) {
		n = strlen(string_to_add);
	}
	*original = realloc(*original, strlen(*original) + n + 1);
	strncat(*original, string_to_add, n);
}


static void _string_array_push(char*** array, char* text, int size) {
	*array = realloc(*array, sizeof(char*) * (size + 2));
	(*array)[size] = text;
	(*array)[size + 1] = NULL;
}


