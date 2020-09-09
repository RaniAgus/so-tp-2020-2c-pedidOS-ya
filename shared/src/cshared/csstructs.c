#include "csstructs.h"

static const char* ESTADO_PEDIDO_STR[] =
{
		"Desconocido",
		"Pendiente",
		"Confirmado",
		"Terminado",
		NULL
};

const char* cs_enum_estado_pedido_to_str(int value)
{
	return ESTADO_PEDIDO_STR[value];
}

void cs_platos_y_estados_destroy(t_list* platos)
{
	void _element_destroyer(t_plato_y_estado* plato)
	{
		free(plato->plato);
		free(plato);
	}
	list_destroy_and_destroy_elements(platos, (void*)_element_destroyer);
}

void cs_receta_destroy(t_list* receta)
{
	void _element_destroyer(t_paso_receta* paso_receta)
	{
		free(paso_receta->paso);
		free(paso_receta);
	}
	list_destroy_and_destroy_elements(receta, (void*)_element_destroyer);
}

t_list* cs_platos_y_estados_create(char* platos, char* listos, char* totales)
{
	t_list* list = list_create();

	//Crea los arrays a iterar (deben tener todos el mismo tamaño)
	char** platos_arr  = string_get_string_as_array(platos);
	char** listos_arr  = string_get_string_as_array(listos);
	char** totales_arr = string_get_string_as_array(totales);

	//Agrega cada línea a una lista
	for(int i=0; platos_arr[i] != NULL; i++)
	{
		t_plato_y_estado* plato_y_estado;

		CHECK_STATUS(MALLOC(plato_y_estado, sizeof(t_plato_y_estado)));

		plato_y_estado->plato = string_duplicate(platos_arr[i]);
		plato_y_estado->cant_lista = atoi(listos_arr[i]);
		plato_y_estado->cant_total = atoi(totales_arr[i]);

		list_add(list, (void*)plato_y_estado);
	}

	//Libera los arrays
	string_iterate_lines(platos_arr, (void*)free);
	free(platos_arr);
	string_iterate_lines(listos_arr, (void*)free);
	free(listos_arr);
	string_iterate_lines(totales_arr, (void*)free);
	free(totales_arr);

	//Retorna la lista
	return list;
}

t_list* cs_receta_create(char* pasos, char* tiempos)
{
	t_list* list = list_create();

	//Crea los arrays a iterar (deben tener todos el mismo tamaño)
	char** pasos_arr   = string_get_string_as_array(pasos);
	char** tiempos_arr = string_get_string_as_array(tiempos);

	//Agrega cada línea a una lista
	for(int i=0; pasos_arr[i] != NULL; i++)
	{
		t_paso_receta* paso_receta;

		CHECK_STATUS(MALLOC(paso_receta, sizeof(t_paso_receta)));

		paso_receta->paso   = string_duplicate(pasos_arr[i]);
		paso_receta->tiempo = atoi(tiempos_arr[i]);

		list_add(list, (void*)paso_receta);
	}

	//Libera los arrays
	string_iterate_lines(pasos_arr, (void*)free);
	free(pasos_arr);
	string_iterate_lines(tiempos_arr, (void*)free);
	free(tiempos_arr);

	//Retorna la lista
	return list;
}
