#ifndef UTILS_MSG_H_
#define UTILS_MSG_H_

#include <stdlib.h>
#include <stdint.h>
#include "cserror.h"
#include "csstring.h"
#include <commons/string.h>
#include "csconn.h"

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

#define CANT_MSGTYPES 15+1

//************************STRUCTS EXTRAS************************

typedef enum
{
	PEDIDO_PENDIENTE = 1,
	PEDIDO_CONFIRMADO,
	PEDIDO_TERMINADO
}e_estado_pedido;

typedef struct
{
	uint32_t x;
	uint32_t y;
}t_pos;

typedef struct
{
	char* 	 plato;
	uint32_t cant_lista;
	uint32_t cant_total;
}t_plato_y_estado;

typedef struct
{
	char* 	 paso;
	uint32_t tiempo;
}t_paso_receta;

//*************************MENSAJES************************

typedef struct
{
	e_msgtype msgtype;

	char* 	 plato;
	uint32_t cantidad;
	char* 	 restaurante;
	uint32_t pedido_id;
}t_mensaje;

#define MENSAJE_PTR(ptr) ((t_mensaje*)(ptr))

typedef enum
{
	MSG_PLATO = 0,
	MSG_CANTIDAD,
	MSG_RESTAURANTE,
	MSG_PEDIDO_ID
}e_msg_elem;

#define MSG_ELEMENTS_CANT 4

//************************RESPUESTAS***************************

//CONSULTAR RESTAURANTES

typedef struct
{
	char** restaurantes;
}t_rta_cons_rest;

#define RTA_CONS_REST(ptr) ((t_rta_cons_rest*)(ptr))

//SELECCIONAR RESTAURANTE (OK/FAIL)

//OBTENER RESTAURANTE

typedef struct
{
	uint32_t cocineros;		 //5
	char** 	 afinidades;	 //[Milanesa,...]
//	char** 	 recetas;		 //[Milanesa,Ensalada,Empanada,...]
	t_pos 	 pos_restaurante;//{posx, posy}
	uint32_t hornos;		 //2
}t_rta_obt_rest;

#define RTA_OBT_REST(ptr) ((t_rta_obt_rest*)(ptr))

//CONSULTAR PLATOS

typedef struct
{
	char** platos;
}t_rta_cons_pl;

#define RTA_CONS_PL(ptr) ((t_rta_cons_pl*)(ptr))

//CREAR PEDIDO

typedef struct
{
	uint32_t pedido_id;
}t_rta_crear_ped;

#define RTA_CREAR_PED(ptr) ((t_rta_crear_ped*)(ptr))

//GUARDAR PEDIDO (OK/FAIL)

//AÑADIR PLATO (OK/FAIL)

//GUARDAR PLATO (OK/FAIL)

//CONFIRMAR PEDIDO (OK/FAIL)

//PLATO LISTO (OK/FAIL)

//CONSULTAR PEDIDO

typedef struct
{
	char* 	 		restaurante;
	e_estado_pedido estado_pedido;
	t_list*			platos_y_estados;
}t_rta_cons_ped;

#define RTA_CONS_PED(ptr)	((t_rta_cons_ped*)(ptr))

//OBTENER_PEDIDO

typedef struct
{
	t_list*	 platos_y_estados;
}t_rta_obt_ped;

#define RTA_OBT_PED(ptr)	((t_rta_obt_ped*)(ptr))

//FINALIZAR PEDIDO (OK/FAIL)

//TERMINAR PEDIDO (OK/FAIL)

//OBTENER RECETA

typedef struct
{
	t_list* pasos_receta;
}t_rta_obt_rec;

#define RTA_OBT_REC(ptr) ((t_rta_obt_rec*)(ptr))

/**
* @NAME cs_enum_msgtype_to_str
* @DESC Devuelve el string correspondiente al enum value de t_enum_msgtype
*/
const char*	cs_enum_msgtype_to_str(int value);

/**
* @NAME cs_msg_destroy
* @DESC Destruye un mensaje y su contenido.
*/
void 	cs_msg_destroy(t_mensaje* msg);

/**
* @NAME cs_rta_destroy
* @DESC Destruye una respuesta y su contenido.
*/
void 	cs_rta_destroy(void* msg, e_msgtype msg_type);

/**
* @NAME cs_receta_destroy
* @DESC Destruye una lista de pasos de una receta.
*/
void 	cs_receta_destroy(t_list* receta);

/**
* @NAME cs_platos_destroy
* @DESC Destruye una lista de platos con sus estados.
*/
void 	cs_platos_destroy(t_list* platos);

/**
* @NAME cs_msg_destroy
* @DESC Crea una estructura mensaje (usar las macros para cada
* tipo de mensaje).
*/
t_mensaje* 	cs_msg_create(e_msgtype msgtype,
						  char*     plato,
						  uint32_t  cant,
						  char*     rest,
						  uint32_t  pedido_id);

#define cs_msg_consultar_rest_create()\
	cs_msg_create(CONSULTAR_RESTAURANTES, "", 0, "", 0)

#define cs_msg_seleccionar_rest_create(rest)\
	cs_msg_create(SELECCIONAR_RESTAURANTE, "", 0, rest, 0)

#define cs_msg_obtener_rest_create(rest)\
	cs_msg_create(OBTENER_RESTAURANTE, "", 0, rest, 0)

#define cs_msg_consultar_pl_create(rest)\
	cs_msg_create(CONSULTAR_PLATOS, "", 0, rest, 0)

#define cs_msg_crear_ped_create()\
	cs_msg_create(CREAR_PEDIDO, "", 0, "", 0)

#define cs_msg_guardar_ped_create(rest, pedido_id)\
	cs_msg_create(GUARDAR_PEDIDO, "", 0, rest, pedido_id)

#define cs_msg_aniadir_pl_create(plato, pedido_id)\
	cs_msg_create(ANIADIR_PLATO, plato, 0, "", pedido_id)

#define cs_msg_guardar_pl_create(plato, cant, rest, pedido_id)\
	cs_msg_create(GUARDAR_PLATO, plato, cant, rest, pedido_id)

#define cs_msg_confirmar_ped_create(pedido_id)\
	cs_msg_create(CONFIRMAR_PEDIDO, "", 0, "", pedido_id)

#define cs_msg_plato_listo_create(plato, rest, pedido_id)\
	cs_msg_create(PLATO_LISTO, plato, 0, rest, pedido_id)

#define cs_msg_consultar_ped_create(pedido_id)\
	cs_msg_create(CONSULTAR_PEDIDO, "", 0, "", pedido_id)

#define cs_msg_obt_ped_create(rest, pedido_id)\
	cs_msg_create(OBTENER_PEDIDO, "", 0, rest, pedido_id)

#define cs_msg_fin_ped_create(rest, pedido_id)\
	cs_msg_create(FINALIZAR_PEDIDO, "", 0, rest, pedido_id)

#define cs_msg_term_ped_create(rest, pedido_id)\
	cs_msg_create(TERMINAR_PEDIDO, "", 0, rest, pedido_id)

#define cs_msg_obtener_receta_create(plato)\
	cs_msg_create(OBTENER_RECETA, plato, 0, "", 0)

/**
* @NAME cs_msg_to_str
* @DESC Recibe un mensaje y devuelve un string con su contenido
* (normalmente para luego mostrarlo por pantalla en un log).
*/
char*		cs_msg_to_str(t_mensaje* msg);

/**
* @NAME cs_rta_to_str
* @DESC Recibe una respuesta y devuelve un string con su contenido
* (normalmente para luego mostrarlo por pantalla en un log).
*/
char*		cs_rta_to_str(void* msg, e_msgtype msg_type);

#endif /* UTILS_MSG_H_ */
