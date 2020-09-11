#ifndef CSHARED_CSSTRUCTS_H_
#define CSHARED_CSSTRUCTS_H_

#include "utils/cscore.h"
#include "utils/cserror.h"

typedef enum
{
	PEDIDO_PENDIENTE = 1,
	PEDIDO_CONFIRMADO,
	PEDIDO_TERMINADO
}e_estado_ped;

typedef struct
{
	uint32_t x;
	uint32_t y;
}t_pos;

typedef struct
{
	char* comida;
	int   cant_lista;
	int   cant_total;
}t_plato;

typedef struct
{
	char* paso;
	int   tiempo;
}t_paso_receta;

typedef struct
{
	char* comida;
	int   precio;
}t_comida_menu;

/**
* @NAME cs_enum_estado_pedido_to_str
* @DESC Devuelve el string correspondiente al enum value de e_estado_ped
*/
const char* cs_enum_estado_pedido_to_str(int value);

/**
* @NAME cs_platos_destroy
* @DESC Destruye una lista de platos con sus estados.
*/
void 	cs_platos_destroy(t_list* platos);

/**
* @NAME cs_receta_destroy
* @DESC Destruye una lista de pasos de una receta.
*/
void 	cs_receta_destroy(t_list* receta);

/**
* @NAME cs_menu_destroy
* @DESC Destruye una lista comidas de un menú.
*/
void	cs_menu_destroy(t_list* menu);

/**
* @NAME cs_platos_create
* @DESC Crea una lista de platos a partir de strings.
* Ejemplo:
* - Entrada:
* 	platos = "[Milanesa,Empanada,Ensalada]"
* 	listos = "[0,2,1]"
* 	totales = "[1,4,1]"
* - Salida:
*   list = [{"Milanesa",0,1},{"Empanada",2,4},{"Ensalada",1,1}]
*/
t_list* cs_platos_create(char* comidas, char* listos, char* totales);

/**
* @NAME cs_receta_create
* @DESC Crea una lista de pasos de una receta a partir de strings.
* Ejemplo:
* - Entrada:
* 	pasos = "[Trozar,Hornear,Reposar]"
* 	tiempos = "[12,20,5]"
* - Salida:
*   list = [{"Trozar",12},{"Hornear",20},{"Reposar",5}]
*/
t_list* cs_receta_create(char* pasos, char* tiempos);

/**
* @NAME cs_menu_create
* @DESC Crea una lista de comidas de un menu a partir de strings.
* Ejemplo:
* - Entrada:
* 	pasos = "[Pizza, Empanada, Milanesa]"
* 	tiempos = "[520,70,240]"
* - Salida:
*   list = [{"Pizza",520},{"Empanada",70},{"Milanesa",240}]
*/
t_list* cs_menu_create(char* comidas, char* precios);

#endif /* CSHARED_CSSTRUCTS_H_ */
