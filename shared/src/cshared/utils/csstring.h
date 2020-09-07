#ifndef UTILS_STRING_H_
#define UTILS_STRING_H_

#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <stdint.h>
#include <ctype.h>

/**
* @NAME cs_stream_copy
* @DESC Copia un stream 'value' a un 'buffer' con un 'offset' (al que le luego se le incrementa la
* cantidad de bytes copiados) o viceversa, dependiendo del flag que se pase por parámetro.
*
* 1 => 'stream' es destino (COPY_SEND) -- 0 => 'value' es destino (COPY_RECV)
*/
void cs_stream_copy(void* stream, int* offset_ptr, void* value, uint32_t value_size,
					int buffer_is_dest);

#define COPY_SEND 1
#define COPY_RECV 0

/**
* @NAME cs_string_to_enum
* @DESC Devuelve el enum value de un string. Si no se encuentra, retorna cero.
*/
int cs_string_to_enum(const char* str, const char* (*enum_to_str)(int));

/**
* @NAME cs_string_to_uint
* @DESC Devuelve por argumento el valor entero de un string (retorna -1 en caso de error).
*/
int cs_string_to_int(int* number, const char* str);

/**
* @NAME cs_string_to_uint
* @DESC Devuelve el valor entero sin signo de un string (-1 en caso de error).
*/
int cs_string_to_uint(const char* str);

/**
* @NAME cs_string_get_as_array (versión de string_get_string_as_array de commons/string.h)
* @DESC Retorna un array separando los elementos de un string con formato de array
*
* Ejemplo:
* char* array_string = "[1,2,3,4]"
* cs_string_get_string_as_array(array_string) => ["1","2","3","4"]
*/
char** cs_string_get_as_array(char* str);

/**
* @NAME cs_string_is_array_format
* @DESC Retorna verdadero si el string pasado por argumento está en formato lista.
*
* Ejemplos: "[1,2,3,4]", "[abc,def,ghi]", "[abc,123,de4,56f]"
*/
bool cs_string_is_array_format(char* str);


/**
* @NAME cs_string_array_lines_count
* @DESC Devuelve la cantidad de líneas que tiene un array de strings
*/
int	cs_string_array_lines_count(char** str_arr);

/**
* @NAME cs_string_array_destroy
* @DESC Libera un array de strings y sus líneas
*/
void cs_string_array_destroy(char** str_arr);

/**
* @NAME cs_string_to_int_array
* @DESC Devuelve por argumento un array de enteros (con o sin signo, dependiendo del flag
* 'is_unsigned'), separando los elementos de un string con formato de array.
* Retorna la cantidad de elementos del array creado, o -1 en caso de error.
*
* Ejemplo:
* char* array_string = "[30,-72,1]"
* cs_string_to_int_array(&intarray, array_string, 0) => intarray = { 30, -72, 1 };
* cs_string_to_int_array(&intarray, array_string, 1) => error;
*/
int cs_string_to_int_array(int** int_arr, char* str, bool is_unsigned);

#endif /* UTILS_STRING_H_ */
