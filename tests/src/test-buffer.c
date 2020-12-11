#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <cshared/utils/csbuffer.h>

#include <cspecs/cspec.h>

typedef struct {
	char* name;
	int   age;
}t_person;

t_person* persona_create(char* name, int age) {
	t_person* new = malloc(sizeof(t_person));
	new->name = strdup(name);
	new->age = age;

	return new;
}

void persona_destroy(t_person* persona) {
	free(persona->name);
	free(persona);
}

void buffer_add_persona(t_buffer* buffer, t_person* person) {
	buffer_pack(buffer, &person->age, sizeof(uint32_t));
	buffer_pack_string(buffer, person->name);
}

t_person* buffer_get_persona(t_buffer* buffer) {
	t_person* persona = malloc(sizeof(t_person));
	buffer_unpack(buffer, &persona->age, sizeof(uint32_t));
	persona->name = buffer_unpack_string(buffer);

	return persona;
}

context (test_buffer) {
	void assert_persona(t_person* got, t_person* expected) {
		should_string(got->name) be equal to (expected->name);
		should_int(got->age) be equal to (expected->age);
	}

	t_buffer* buffer;

	describe("packing numbers") {
		uint32_t integers[3] = { 1, 2, 3 };

		before {
			buffer = buffer_create();
			for(int i = 0; i < 3; i++) {
				buffer_pack(buffer, &integers[i], sizeof(uint32_t));
			}
		} end

		after {
			buffer_destroy(buffer);
		} end

		it("should pack multiple integers") {
			should_int(buffer->size) be equal to (12);
		} end

		it("should unpack multiple integers in same order as packed") {
			uint32_t result[3];
			for(int i = 0; i < 3; i++) {
				buffer_unpack(buffer, &result[i], sizeof(uint32_t));
				should_int(result[i]) be equal to (integers[i]);
			}	
			should_int(buffer->size) be equal to (0);
		} end
	} end

	describe("packing strings") {
		char* strings[] = {"Agustin", "Matias", "Gaston", "Sebastian", "Daniela"};

		before {
			buffer = buffer_create();
			for(int i = 0; i < 5; i++) {
				buffer_pack_string(buffer, strings[i]);
			}
		} end

		after {
			buffer_destroy(buffer);
		} end
		
		it("should pack multiple strings including each size upfront") {
			should_int(buffer->size) be equal to (55);
		} end
		
		it("should unpack multiple strings in same order as packed") {
			for(int i = 0; i < 5; i++) {
				char* result = buffer_unpack_string(buffer);
				should_string(result) be equal to (strings[i]);
				free(result);
			}
			should_int(buffer->size) be equal to (0);
		} end

	} end

	describe("packing string arrays") {
		char* strings[] = {"Agustin", "Matias", "Gaston", "Sebastian", "Daniela", NULL};

		before {
			buffer = buffer_create();
			buffer_pack_string_array(buffer, strings);
		} end

		after {
			buffer_destroy(buffer);
		} end

		it("should pack array as string including size upfront") {
			should_int(buffer->size) be equal to (45);
		} end

		it("should unpack array in same order as packed") {
			char** result = buffer_unpack_string_array(buffer);

			int i = 0;
			void assert_strings(char* line) {
				should_string(line) be equal to (strings[i++]);
				free(line);
			}
			string_iterate_lines(result, assert_strings);
			free(result);

			should_int(buffer->size) be equal to (0);
		} end
	} end

	describe("packing lists") {
		t_list* list;

		before {
			list = list_create();
			list_add(list, persona_create("Agustin"  , 21));
			list_add(list, persona_create("Matias"   , 24));
			list_add(list, persona_create("Gaston"   , 25));
			list_add(list, persona_create("Sebastian", 21));
			list_add(list, persona_create("Daniela"  , 19));

			buffer = buffer_create();
			buffer_pack_list(buffer, list, (void*) buffer_add_persona);
		} end

		after {
			buffer_destroy(buffer);
			list_destroy_and_destroy_elements(list, (void*) persona_destroy);
		} end

		it("should pack a list including elements count upfront") {
			should_int(buffer->size) be equal to (79);
		} end

		it("should unpack a list in the same order as packed") {
			t_list* other = buffer_unpack_list(buffer, (void*) buffer_get_persona);
			for(int i = 0; i < 5; i++) {
				assert_persona(list_get(other, i), list_get(list, i));
			}
			should_int(buffer->size) be equal to (0);
			list_destroy_and_destroy_elements(other, (void*) persona_destroy);
		} end

	} end
}
