#include "cslist.h"

int list_find_index(t_list* self, bool(*condition)(void*)) {
	t_link_element* element = self->head;
	int index = 0;

	while(element != NULL) {
		if(condition(element->data)) {
			return index;
		}
		element = element->next;
		index++;
	}

	return -1;
}
