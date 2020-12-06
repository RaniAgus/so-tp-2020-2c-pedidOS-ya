#include "csbuffer.h"

t_buffer* buffer_create() {
	t_buffer* buffer = malloc(sizeof(t_buffer));

	buffer->size = 0;
	buffer->stream = NULL;

	return buffer;
}

void buffer_pack(t_buffer* buffer, void* stream_to_add, int size) {
	buffer->stream = realloc(buffer->stream, buffer->size + size);
	memcpy(buffer->stream + buffer->size, stream_to_add, size);
	buffer->size += size;
}

void buffer_pack_string(t_buffer* buffer, char* string_to_add) {
	uint32_t length = strlen(string_to_add);
	buffer_pack(buffer, &length, sizeof(uint32_t));

	buffer->stream = realloc(buffer->stream, buffer->size + length);
	memcpy(buffer->stream + buffer->size, string_to_add, length);
	buffer->size += length;
}

void buffer_pack_string_array(t_buffer* buffer, char** string_array_to_add) {
	char* strings = cs_string_array_to_string(string_array_to_add);
	buffer_pack_string(buffer, strings);
	free(strings);
}

void buffer_pack_list(t_buffer* buffer, t_list* src, void(*element_packer)(t_buffer*, void*)) {
	uint32_t length = src->elements_count;
	buffer_pack(buffer, &length, sizeof(uint32_t));

	void _add_elements_to_buffer(void* element) {
		element_packer(buffer, element);
	}
	list_iterate(src, _add_elements_to_buffer);
}

void buffer_unpack(t_buffer* buffer, void* dest, int size) {
	memcpy(dest, buffer->stream, size);
	buffer->size -= size;
	memmove(buffer->stream, buffer->stream + size, buffer->size);
	buffer->stream = realloc(buffer->stream, buffer->size);
}

char* buffer_unpack_string(t_buffer* buffer) {
	char* dest;
	uint32_t length;

	buffer_unpack(buffer, &length, sizeof(uint32_t));
	dest = calloc(length + 1, sizeof(char));
	buffer_unpack(buffer, dest, length);

	return dest;
}

char** buffer_unpack_string_array(t_buffer* buffer) {
	char* text = buffer_unpack_string(buffer);
	char** array = string_get_string_as_array(text);
	free(text);

	return array;
}

t_list* buffer_unpack_list(t_buffer* buffer, void*(*element_unpacker)(t_buffer*)) {
	t_list* list = list_create();
	uint32_t length;

	buffer_unpack(buffer, &length, sizeof(uint32_t));
	for(uint32_t i = 0; i < length; i++) {
		list_add(list, element_unpacker(buffer));
	}

	return list;
}

void buffer_destroy(t_buffer* buffer) {
	if(buffer->size > 0) {
		free(buffer->stream);
	}
	free(buffer);
}
