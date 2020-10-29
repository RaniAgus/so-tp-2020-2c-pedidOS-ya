#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cshared/utils/csstring.h>

#include <cspecs/cspec.h>

typedef enum { ENUM_ERROR, ENUM_ONE, ENUM_TWO, ENUM_THREE } e_example;

const char* ENUM_STR[] = { "Error", "One", "Two", "Three", NULL };

const char* enum_to_string_func(int value) {
	return ENUM_STR[value];
}

context (test_string) {
	describe("string to enum") {
		it("enum found"){
			e_example result = cs_string_to_enum("Two", enum_to_string_func);
			should_int(result) be equal to (ENUM_TWO);
		} end

		it("enum not found"){
			e_example result = cs_string_to_enum("Four", enum_to_string_func);
			should_int(result) be equal to (ENUM_ERROR);
		} end
	} end

	describe("string to unsigned int") {
		it("valid unsigned int") {
			int result = cs_string_to_uint("1234567890");
			should_int(result) be equal to (1234567890);
		} end

		it("invalid unsigned int") {
			int result = cs_string_to_uint("-32");
			should_int(result) be equal to (-1);
		} end

	} end

	describe("is string array"){
		it("is in string array format"){
			bool result = cs_string_is_string_array("[1,2,3]");
			should_bool(result) be truthy;
		} end

		it("isn't in string array format having empty values at the end"){
			bool result1 = cs_string_is_string_array("[1,]");
			should_bool(result1) be falsey;
		} end

		it("isn't in string array format having empty values at the beggining"){
			bool result2 = cs_string_is_string_array("[,1]");
			should_bool(result2) be falsey;
		} end

		it("isn't in string array format having empty values in between"){
			bool result3 = cs_string_is_string_array("[1,,2]");
			should_bool(result3) be falsey;
		} end

		it("isn't in string array format if it doesn't end with brackets"){
			bool result4 = cs_string_is_string_array("1]");
			should_bool(result4) be falsey;
		} end

		it("isn't in string array format if it doesn't start with brackets"){
			bool result5 = cs_string_is_string_array("[1");
			should_bool(result5) be falsey;
		} end
	} end

	describe("is unsigned int array") {
		it("string is in unsigned int array format") {
			bool result = cs_string_is_unsigned_int_array("[1,2,3]");
			should_bool(result) be truthy;
		} end

		it("string isn't in unsigned int array format"){
			bool result = cs_string_is_unsigned_int_array("[1,2a,3]");
			should_bool(result) be falsey;
		} end
	} end

	it("string array to string") {
		char* result = cs_string_array_to_string((char**)ENUM_STR);
		should_string(result) be equal to ("[Error,One,Two,Three]");
		free(result);
	} end

	describe ("string array") {
		char** names;

		before {
			names = string_array_new();

			string_array_push(&names, "Gaston");
			string_array_push(&names, "Matias");
			string_array_push(&names, "Sebastian");
			string_array_push(&names, "Daniela");
		}end

		after {
			free(names);
		}end

		it ("add an element at the end") {
			string_array_push(&names, "Agustin");

			should_int(string_array_size(names)) be equal to (5);
			should_ptr(names[5]) be null;

			char* expected[] = {"Gaston", "Matias", "Sebastian", "Daniela", "Agustin"};
			int i = 0;
			void _assert_names(char* name) {
				should_ptr(name) not be null;
				should_string(name) be equal to (expected[i]);
				i++;
			}
			string_iterate_lines(names, _assert_names);
		}end

	}end
}
