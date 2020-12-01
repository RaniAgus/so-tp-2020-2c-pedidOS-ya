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
void cs_stream_copy(void* stream, int* offset_ptr, void* value, uint32_t value_size, bool buffer_is_dest);

#define COPY_SEND 1
#define COPY_RECV 0

/**
* @NAME cs_string_to_enum
* @DESC Devuelve el enum value de un string. Si no se encuentra, retorna cero.
*/
int cs_string_to_enum(const char* str, const char* (*enum_to_str)(int));

/**
* @NAME cs_string_to_uint
* @DESC Devuelve el valor entero sin signo de un string (-1 en caso de error).
*/
int cs_string_to_uint(const char* str);

/**
* @NAME cs_string_is_in_string_array_format
* @DESC Retorna != 0 si el string pasado por argumento está en formato array.
*
* Ejemplos válidos:
* 	"[1,2,3,4]", "[abc,def,ghi]", "[abc,123,de4,56f]", "[abcd]"
* Ejemplos no válidos:
* 	"[a,b,c", "[a,b,,c]", "a,b,c]"
*/
bool cs_string_is_string_array(char* str);

/**
* @NAME cs_string_is_in_uint_array_format
* @DESC Retorna != 0 si el string pasado por argumento está en formato array
* de enteros sin signo.
*
* Ejemplos válidos:
* 	"[1,2,3,4]", "[1234]"
* Ejemplos NO válidos:
* 	"[a,0,1]", "[0,1,2", "[0,1,,2]", "0,1,2]", "[-2,-1,0,1,2]"
*/
bool cs_string_is_unsigned_int_array(char* str);

/**
* @NAME cs_string_array_to_string
* @DESC Devuelve un string en formato array, a partir de un array de strings
*/
char* cs_string_array_to_string(char** str_arr);

/**
* @NAME: string_new
* @DESC: Crea un array de strings vacio
*/
char** string_array_new();

/**
* @NAME string_array_size
* @DESC Devuelve la cantidad de líneas que tiene un array de strings
*/
int	string_array_size(char** str_arr);

/*
 * @NAME: string_array_is_empty
 * @DESC: Verifica si el array de strings está vacío
 */
bool string_array_is_empty(char** array);

/**
* @NAME: string_array_push
* @DESC: Agrega un string al final del array
*/
void string_array_push(char*** array, char* text);

/**
* @NAME: string_n_append
* @DESC: Agrega al primer string un máximo de n caracteres
* del segundo.
*
* Ejemplo:
* char *unaPalabra = string_new();
* string_n_append(&unaPalabra, "HOLA ", 10);
* string_n_append(&unaPalabra, "PEPE", 3);
*
* => unaPalabra = "HOLA PEP"
*/
void string_n_append(char** original, char* string_to_add, int n);

#endif /* UTILS_STRING_H_ */
