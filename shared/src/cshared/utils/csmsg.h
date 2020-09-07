#ifndef UTILS_MSG_H_
#define UTILS_MSG_H_

#include <stdlib.h>
#include <stdint.h>
#include "cserror.h"
#include "csstring.h"
#include <commons/string.h>

typedef enum
{
	MSGTYPE_UNKNOWN = 0,
	CONSULTAR_RESTAURANTES,
	SELECCIONAR_RESTAURANTE,
	OBTENER_RESTAURANTE,
	CONSULTAR_PLATOS,
	CREAR_PEDIDO,
	GUARDAR_PEDIDO,
	ANIADIR_PLATO,
	GUARDAR_PLATO,
	CONFIRMAR_PEDIDO,
	PLATO_LISTO,
	CONSULTAR_PEDIDO,
	OBTENER_PEDIDO,
	FINALIZAR_PEDIDO,
	TERMINAR_PEDIDO,
	OBTENER_RECETA
}e_msgtype;

typedef struct
{
	uint32_t x;
	uint32_t y;
}t_pos;

//****************CONSULTAR RESTAURANTES

typedef struct
{
	char* restaurantes;
}t_res_consultar_rest;

//****************SELECCIONAR RESTAURANTE

typedef struct
{
	char* nombre_rest;
	//TODO: Cliente??
}t_msg_seleccionar_rest;

//****************OBTENER RESTAURANTE y CONSULTAR PLATOS

typedef struct
{
	char* nombre_rest;
}t_msg_obtener_rest, t_msg_consultar_platos;

typedef struct
{
	uint32_t cocineros;	//5
	char* 	 afinidades;//[Milanesa,...]
	t_pos 	 posicion;	//{posx, posy}
//	char* 	 recetas;	//[Milanesa,Ensalada,Empanada,...]
	uint32_t hornos;	//2
}t_res_obtener_rest;

//TODO: Esperar respuesta
typedef struct
{
	char* platos;
}t_res_consultar_platos;

//****************CREAR PEDIDO

typedef struct
{
	uint32_t pedido_id;
}t_res_crear_pedido;

//****************GUARDAR PEDIDO

typedef struct
{
	uint32_t pedido_id;
	char* 	 nombre_rest;
}t_msg_guardar_pedido;

//*****************AÑADIR PLATO

typedef struct
{
	uint32_t pedido_id;
	char* 	 plato;
}t_msg_aniadir_plato;

//*****************PLATO LISTO

typedef struct
{
	uint32_t pedido_id;
	char*	 restaurante;
	char*	 plato;
}t_msg_plato_listo;

//*******************CONSULTAR_PEDIDO y CONFIRMAR PEDIDO

typedef struct
{
	uint32_t pedido_id;
}t_msg_consultar_pedido,
 t_msg_confirmar_pedido;

typedef enum
{
	PEDIDO_PENDIENTE = 1,
	PEDIDO_CONFIRMADO,
	PEDIDO_TERMINADO
}e_estado_pedido;

typedef struct
{
	char* nombre;
	uint32_t cant_lista;
	uint32_t cant_total;
}t_plato;

typedef struct
{
	char* 	 		restaurante;
	e_estado_pedido estado;
	t_list*			platos;
}t_res_consultar_pedido;

//*******************OBTENER_PEDIDO

typedef struct
{
	uint32_t pedido_id;
	char*	 restaurante;
}t_msg_obtener_pedido;

typedef struct
{
	t_list*	 platos;
}t_res_obtener_pedido;

//*******************

//*****************GUARDAR PLATO, FINALIZAR_PEDIDO y TERMINAR PEDIDO

typedef struct
{
	uint32_t pedido_id;
	char*	 restaurante;
}t_msg_guardar_plato,
 t_msg_finalizar_pedido,
 t_msg_finalizar_pedido;

//*******************

typedef t_msg_finalizar_pedido t_msg_terminar_pedido

//*******************OBTENER RECETA

typedef struct
{
	char* plato;
}t_msg_obtener_receta;

typedef struct
{
	char* receta;
}t_res_obtener_receta;


/**
* @NAME cs_enum_msgtype_to_str
* @DESC Devuelve el string correspondiente al enum value de t_enum_msgtype
*/
const char* cs_enum_msgtype_to_str(int value);

/**
* @NAME cs_msg_destroy
* @DESC Destruye un mensaje y su contenido.
*/
void 		cs_msg_destroy(void* msg, e_msgtype msg_type);

/**
* @NAME cs_msg_to_str
* @DESC Recibe un mensaje y devuelve un string con su contenido
* (normalmente para luego mostrarlo por pantalla en un log).
*/
char* 		cs_msg_to_str(void* msg, e_msgtype msgtype);

#endif /* UTILS_MSG_H_ */
