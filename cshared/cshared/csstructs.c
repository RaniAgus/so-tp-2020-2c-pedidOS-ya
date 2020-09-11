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

void cs_platos_destroy(t_list* platos)
{
	void _element_destroyer(t_plato* plato)
	{
		free(plato->comida);
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

void cs_menu_destroy(t_list* menu)
{
	void _element_destroyer(t_comida_menu* plato_menu)
	{
		free(plato_menu->comida);
		free(plato_menu);
	}
	list_destroy_and_destroy_elements(menu, (void*)_element_destroyer);
}

t_list* cs_platos_create(char* comidas, char* listos, char* totales)
{
	t_list* list = list_create();

	//Crea los arrays a iterar (deben tener todos la misma cantidad de elementos)
	char** platos_arr  = string_get_string_as_array(comidas);
	char** listos_arr  = string_get_string_as_array(listos);
	char** totales_arr = string_get_string_as_array(totales);

	//Agrega cada línea a una lista
	for(int i=0; platos_arr[i] != NULL; i++)
	{
		t_plato* plato_y_estado;

		CHECK_STATUS(MALLOC(plato_y_estado, sizeof(t_plato)));

		plato_y_estado->comida = string_duplicate(platos_arr[i]);
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

	//Crea los arrays a iterar (deben tener todos la misma cantidad de elementos)
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

t_list* cs_menu_create(char* comidas, char* precios)
{
	t_list* list = list_create();

	//Crea los arrays a iterar (deben tener todos la misma cantidad de elementos)
	char** comidas_arr = string_get_string_as_array(comidas);
	char** precios_arr = string_get_string_as_array(precios);

	//Agrega cada línea a una lista
	for(int i=0; comidas_arr[i] != NULL; i++)
	{
		t_comida_menu* comida_menu;

		CHECK_STATUS(MALLOC(comida_menu, sizeof(t_paso_receta)));

		comida_menu->comida = string_duplicate(comidas_arr[i]);
		comida_menu->precio = atoi(precios_arr[i]);

		list_add(list, (void*)comida_menu);
	}

	//Libera los arrays
	string_iterate_lines(comidas_arr, (void*)free);
	free(comidas_arr);
	string_iterate_lines(precios_arr, (void*)free);
	free(precios_arr);

	//Retorna la lista
	return list;
}
