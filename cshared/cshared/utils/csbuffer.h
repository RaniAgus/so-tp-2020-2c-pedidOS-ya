#ifndef BUFFER_H_
#define BUFFER_H_

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <stdbool.h>
	#include <stdint.h>
	#include <commons/collections/list.h>

	#include "csstring.h"

	typedef struct {
		uint32_t size;
		void*    stream;
	}t_buffer;

	/**
	* @NAME buffer_create
	* @DESC Inicializa una estructura buffer.
	*/
	t_buffer* buffer_create();

	/**
	* @NAME buffer_pack
	* @DESC Concatena un stream de un tamaño fijo en bytes
	* al final del stream del buffer.
	*/
	void buffer_pack(t_buffer* buffer, void* src, int size);

	/**
	* @NAME buffer_pack_string
	* @DESC Concatena al final del buffer un string detrás de
	* un uint32_t que indique su longitud en bytes.
	*/
	void buffer_pack_string(t_buffer* buffer, char* src);

	/**
	* @NAME buffer_pack_string_array
	* @DESC Concatena al final del buffer un array de strings
	* convertido a string, detrás de un uint32_t que indique su
	* longitud en bytes.
	* ADVERTENCIA: Los strings NO deben tener comas (',').
	*/
	void buffer_pack_string_array(t_buffer* buffer, char** string_array_to_add);

	/**
	* @NAME buffer_pack_list
	* @DESC Concatena al final del buffer los elementos de una lista
	* convertidos a buffer, detrás de un uint32_t que indique el tamaño de la lista.
	* 
	* 'element_packer(2)' empaqueta cada elemento de la lista en el buffer, 
	* ver el código de 'buffer_pack_string(2)' para encontrar un ejemplo
	*/
	void buffer_pack_list(t_buffer* buffer, t_list* src, void(*element_packer)(t_buffer*, void*));

	/**
	* @NAME buffer_unpack
	* @DESC Extrae del principio del stream del buffer un fragmento
	* del mismo y lo ubica en el stream destino.
	*/
	void buffer_unpack(t_buffer* buffer, void* dest, int size);

	/**
	* @NAME buffer_unpack_string
	* @DESC Extrae del principio del stream del buffer un string (junto con
	* el uint32_t que indica su tamaño) y lo retorna
	*/
	char* buffer_unpack_string(t_buffer* buffer);

	/**
	* @NAME buffer_unpack_string_array
	* @DESC Extrae del principio del stream del buffer un string en formato array
	* (junto con el uint32_t que indica su tamaño) y lo retorna
	*/
	char** buffer_unpack_string_array(t_buffer* buffer);

	/**
	* @NAME buffer_unpack_list
	* @DESC Extrae del principio del stream del buffer todos los elementos de una lista
	* (junto con el uint32_t que indica su tamaño) y la retorna.
	*
	* 'element_unpacker(1)' desempaqueta cada elemento de la lista en el buffer, 
	* ver el código de 'buffer_unpack_string(1)' para encontrar un ejemplo
	*/
	t_list* buffer_unpack_list(t_buffer* buffer, void*(*element_unpacker)(t_buffer*));

	/**
	* @NAME buffer_destroy
	* @DESC Destruye una estructura buffer y su contenido
	* en caso de tenerlo.
	*/
	void buffer_destroy(t_buffer* buffer);

#endif /* BUFFER_H_ */
