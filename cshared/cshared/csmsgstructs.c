#include "csmsgstructs.h"

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

	if(platos_arr)
	{
		//Agrega cada línea a una lista
		for(int i=0; platos_arr[i] != NULL; i++)
		{
			t_plato* plato_y_estado;

			plato_y_estado = malloc(sizeof(t_plato));

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
	}

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

		paso_receta = malloc(sizeof(t_paso_receta));

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

		comida_menu = malloc(sizeof(t_paso_receta));

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

void cs_platos_to_string(t_list* platos, char** comidas, char** listos, char** totales)
{
	//Inicia los array de comidas y estados
	*comidas = string_duplicate("[");
	*listos  = string_duplicate("[");
	*totales = string_duplicate("[");

	if(platos->elements_count > 0)
	{
		//Itera la lista, agregando la comida y el estado al respectivo string
		void _platos_to_string(t_plato* plato)
		{
			string_append_with_format(comidas, "%s,", plato->comida);
			string_append_with_format(listos , "%d,", plato->cant_lista);
			string_append_with_format(totales, "%d,", plato->cant_total);
		}
		list_iterate(platos, (void*)_platos_to_string);

		//Corrige el corchete al final
		(*comidas)[strlen(*comidas) - 1] = ']';
		(*listos )[strlen(*listos ) - 1] = ']';
		(*totales)[strlen(*totales) - 1] = ']';
	} else
	{
		//Lista vacía
		string_append(comidas, "]");
		string_append(listos , "]");
		string_append(totales, "]");
	}
}

void cs_receta_to_string(t_list* receta, char** pasos, char** tiempos)
{
	//Inicia los array de pasos y tiempos
	*pasos   = string_duplicate("[");
	*tiempos = string_duplicate("[");

	//Itera la lista, agregando el nombre del paso y el tiempo al respectivo string
	void _receta_to_string(t_paso_receta* paso_receta)
	{
		string_append_with_format(pasos,   "%s,", paso_receta->paso);
		string_append_with_format(tiempos, "%d,", paso_receta->tiempo);
	}
	list_iterate(receta, (void*)_receta_to_string);

	//Corrige la coma al final
	(*pasos  )[strlen(*pasos  ) - 1] = ']';
	(*tiempos)[strlen(*tiempos) - 1] = ']';
}

void cs_menu_to_string(t_list* menu, char** comidas, char** precios)
{
	//Inicia los array de comidas y precios
	*comidas = string_duplicate("[");
	*precios = string_duplicate("[");

	//Itera la lista, agregando la comida y el precio al respectivo string
	void _menu_to_string(t_comida_menu* comida_menu)
	{
		string_append_with_format(comidas, "%s,", comida_menu->comida);
		string_append_with_format(precios, "%d,", comida_menu->precio);
	}
	list_iterate(menu, (void*)_menu_to_string);

	//Corrige la coma al final
	(*comidas)[strlen(*comidas) - 1] = ']';
	(*precios)[strlen(*precios) - 1] = ']';
}

int cs_platos_sumar_listos(t_list* platos)
{
	double _sumar_listos(t_plato* plato){
		return (double)plato->cant_lista;
	}

	return (int)list_sum(platos, (void*) _sumar_listos);
}

int cs_platos_sumar_totales(t_list* platos)
{
	double _sumar_totales(t_plato* plato){
		return (double)plato->cant_total;
	}

	return (int)list_sum(platos, (void*) _sumar_totales);
}

t_list* cs_receta_duplicate(t_list* receta)
{
	t_list* duplicate = list_create();

	void _duplicar_pasos(t_paso_receta* plato)
	{
		t_paso_receta* nuevo = malloc(sizeof(t_paso_receta));
		nuevo->paso = strdup(plato->paso);
		nuevo->tiempo = plato->tiempo;

		list_add(duplicate, nuevo);
	}
	list_iterate(receta, (void*) _duplicar_pasos);

	return duplicate;
}

double calcular_norma(t_pos vector)
{
	double a = pow(vector.x, 2);
	double b = pow(vector.y, 2);
	double norma = sqrt(a + b);
	return norma;
}

t_pos calcular_vector_distancia(t_pos posicion1, t_pos posicion2)
{
	t_pos distancia;
	distancia.x = posicion1.x - posicion2.x;
	distancia.y = posicion1.y - posicion2.y;
	return distancia;
}
