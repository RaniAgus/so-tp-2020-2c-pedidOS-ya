#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cshared/utils/csutils.h>

#include <cspecs/cspec.h>

typedef struct {
    char *name;
    unsigned char age;
} t_person;

static t_person *persona_create(char *name, unsigned char age) {
    t_person *new = malloc(sizeof(t_person));
    new->name = strdup(name);
    new->age = age;
    return new;
}

static void persona_destroy(t_person *self) {
    free(self->name);
    free(self);
}

static int _ayudantes_menor(t_person *joven, t_person *menos_joven) {
    return joven->age - menos_joven->age;
}

static int _ayudantes_alfabetico(t_person *primero, t_person *segundo) {
    return strcmp(primero->name, segundo->name);
}

context (test_utils) {
    void assert_person(t_person *person, char* name, int age) {
        should_ptr(person) not be null;
        should_string(person->name) be equal to(name);
        should_int(person->age) be equal to(age);
    }

    void assert_person_in_list(t_list *list, int index, char* name, int age) {
        assert_person(list_get(list, index), name, age);
    }

	describe("Lists") {
	    t_list *list;

        before {
	    	list = list_create();
            list_add(list, persona_create("Nicolas", 6));
            list_add(list, persona_create("Matias", 70));
            list_add(list, persona_create("Juan", 124));
            list_add(list, persona_create("Juan Manuel", 1));
            list_add(list, persona_create("Sebastian", 8));
            list_add(list, persona_create("Rodrigo", 40));
        } end

		after {
        	list_destroy_and_destroy_elements(list, (void*) persona_destroy);
        } end

		it("should fold all values into a single one, starting with first element") {
        	t_person* get_oldest_person(t_person* person1, t_person* person2) {
                return person1->age >= person2->age ? person1 : person2;
            }

            t_person* oldestPerson = (t_person*) list_fold1(list, (void*) get_oldest_person);

            assert_person(oldestPerson, "Juan", 124);
        } end

		it("should fold an empty list") {
        	list_clean_and_destroy_elements(list, (void*) persona_destroy);

            t_person* get_oldest_person(t_person* person1, t_person* person2) {
                return person1->age >= person2->age ? person1 : person2;
            }

            t_person* oldestPerson = (t_person*) list_fold1(list, (void*) get_oldest_person);

            should_ptr(oldestPerson) be null;
        } end

		it("should get minimum") {
        	t_person* youngestPerson = (t_person*) list_get_min_by(list, (void*)_ayudantes_menor);

            assert_person(youngestPerson, "Juan Manuel", 1);
        } end

        it("should get maximum") {
        	t_person* lastAlphabetical = (t_person*) list_get_max_by(list, (void*)_ayudantes_alfabetico);

            assert_person(lastAlphabetical, "Sebastian", 8);
        } end

    } end

}
