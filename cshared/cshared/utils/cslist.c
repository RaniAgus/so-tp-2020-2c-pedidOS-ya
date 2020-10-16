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

double list_sum(t_list* self, double(*element_value)(void*)) {
	t_link_element* element = self->head;
	double sumatory = 0;

	while(element != NULL) {
		sumatory += element_value(element->data);
	}

	return sumatory;
}
