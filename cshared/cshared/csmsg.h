#ifndef UTILS_MSG_H_
#define UTILS_MSG_H_

#include <stdlib.h>
#include <stdint.h>
#include <commons/string.h>
#include "utils/cscore.h"
#include "utils/cserror.h"
#include "utils/csstring.h"
#include "csconn.h"
#include "csstructs.h"

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

#define MSGTYPES_CANT 15+1

/**
* @NAME cs_enum_msgtype_to_str
* @DESC Devuelve el string correspondiente al enum value de e_msgtype
*/
const char*	cs_enum_msgtype_to_str(int value);

/**
* @NAME cs_msg_to_str
* @DESC Recibe un mensaje y devuelve un string con su contenido
* (normalmente para luego mostrarlo por pantalla en un log).
*/
char* 	cs_msg_to_str(void* msg, e_opcode op_code, e_msgtype msg_type);

/**
* @NAME cs_msg_destroy
* @DESC Destruye un mensaje (o respuesta) y su contenido.
*/
void 	cs_msg_destroy(void* msg, e_opcode op_code, e_msgtype msg_type);

//*************************MENSAJES************************

typedef struct
{
	e_msgtype msgtype;

	char* 	 comida;
	uint32_t cantidad;
	char* 	 restaurante;
	uint32_t pedido_id;
}t_solicitud;

#define SOLICITUD_PTR(ptr) ((t_solicitud*)(ptr))

typedef enum
{
	SOL_ARG_COMIDA = 0,
	SOL_ARG_CANTIDAD,
	SOL_ARG_RESTAURANTE,
	SOL_ARG_PEDIDO_ID
}e_sol_arg;

#define SOL_ARGS_CANT 4

/**
* @NAME cs_sol_has_argument
* @DESC Devuelve un boolean indicando si el tipo de solicitud
* tiene ese argumento.
*/
bool cs_sol_has_argument(e_msgtype msgtype, e_sol_arg arg);

t_solicitud* _sol_create(e_msgtype msgtype, char* plato, uint32_t cant, char* rest, uint32_t pedido_id);

/**
* @NAME cs_sol/rta_create
* @DESC Crean una estructura solicitud o respuesta.
*/

//*************************CONSULTAR RESTAURANTES*************************

#define cs_msg_consultar_rest_create()\
	cs_msg_create(CONSULTAR_RESTAURANTES, "", 0, "", 0)

typedef struct
{
	char** restaurantes;
}t_rta_cons_rest;

#define RTA_CONS_REST(ptr) ((t_rta_cons_rest*)(ptr))

t_rta_cons_rest* cs_rta_consultar_rest_create(char* restaurantes);

//*************************SELECCIONAR RESTAURANTE*************************

#define cs_msg_seleccionar_rest_create(rest)\
		_sol_create(SELECCIONAR_RESTAURANTE, "", 0, rest, 0)

//*************************OBTENER RESTAURANTE*************************

#define cs_msg_obtener_rest_create(rest)\
		_sol_create(OBTENER_RESTAURANTE, "", 0, rest, 0)

typedef struct
{
	uint32_t cant_cocineros;
	char** 	 afinidades;
	t_list*  menu;
	t_pos 	 pos_restaurante;
	uint32_t cant_hornos;
}t_rta_obt_rest;

#define RTA_OBT_REST(ptr) ((t_rta_obt_rest*)(ptr))

t_rta_obt_rest* cs_rta_obtener_rest_create(uint32_t cant_cocineros,
								   	   	   char* 	afinidades,
										   char*	comidas,
										   char*	precios,
										   t_pos 	pos_restaurante,
										   uint32_t cant_hornos);

//*************************CONSULTAR PLATOS*************************

#define cs_msg_consultar_pl_create(rest)\
		_sol_create(CONSULTAR_PLATOS, "", 0, rest, 0)

typedef struct
{
	char** platos;
}t_rta_cons_pl;

#define RTA_CONS_PL(ptr) ((t_rta_cons_pl*)(ptr))

t_rta_cons_pl* cs_rta_consultar_pl_create(char* platos);

//*************************CREAR PEDIDO*************************

#define cs_msg_crear_ped_create()\
		_sol_create(CREAR_PEDIDO, "", 0, "", 0)

typedef struct
{
	uint32_t pedido_id;
}t_rta_crear_ped;

#define RTA_CREAR_PED(ptr) ((t_rta_crear_ped*)(ptr))

t_rta_crear_ped* cs_rta_crear_ped_create(uint32_t pedido_id);

//*************************GUARDAR PEDIDO*************************

#define cs_msg_guardar_ped_create(rest, pedido_id)\
		_sol_create(GUARDAR_PEDIDO, "", 0, rest, pedido_id)

//*************************AÑADIR PLATO*************************

#define cs_msg_aniadir_pl_create(plato, pedido_id)\
		_sol_create(ANIADIR_PLATO, plato, 0, "", pedido_id)

//*************************GUARDAR PLATO*************************

#define cs_msg_guardar_pl_create(plato, cant, rest, pedido_id)\
		_sol_create(GUARDAR_PLATO, plato, cant, rest, pedido_id)

//*************************CONFIRMAR PEDIDO*************************

#define cs_msg_confirmar_ped_create(pedido_id)\
		_sol_create(CONFIRMAR_PEDIDO, "", 0, "", pedido_id)

//*************************PLATO LISTO*************************

#define cs_msg_plato_listo_create(plato, rest, pedido_id)\
		_sol_create(PLATO_LISTO, plato, 0, rest, pedido_id)

//*************************CONSULTAR PEDIDO*************************

#define cs_msg_consultar_ped_create(pedido_id)\
	cs_msg_create(CONSULTAR_PEDIDO, "", 0, "", pedido_id)

typedef struct
{
	char* 	 		restaurante;
	e_estado_ped 	estado_pedido;
	t_list*			platos_y_estados;
}t_rta_cons_ped;

#define RTA_CONSULTAR_PED(ptr)	((t_rta_cons_ped*)(ptr))

t_rta_cons_ped* cs_rta_consultar_ped_create(char* rest,
											e_estado_ped estado_ped,
											char* platos,
											char* listos,
											char* totales);

//*************************OBTENER PEDIDO*************************

#define cs_msg_obtener_ped_create(rest, pedido_id)\
		_sol_create(OBTENER_PEDIDO, "", 0, rest, pedido_id)

typedef struct
{
	t_list*	 platos_y_estados;
}t_rta_obt_ped;

#define RTA_OBTENER_PED(ptr)	((t_rta_obt_ped*)(ptr))

t_rta_obt_ped* cs_rta_obtener_ped_create(char* platos, char* listos, char* totales);

//*************************FINALIZAR PEDIDO*************************

#define cs_msg_fin_ped_create(rest, pedido_id)\
		_sol_create(FINALIZAR_PEDIDO, "", 0, rest, pedido_id)

//*************************TERMINAR PEDIDO*************************

#define cs_msg_term_ped_create(rest, pedido_id)\
		_sol_create(TERMINAR_PEDIDO, "", 0, rest, pedido_id)

//*************************OBTENER RECETA*************************

#define cs_msg_rta_obtener_receta_create(plato)\
		_sol_create(OBTENER_RECETA, plato, 0, "", 0)

typedef struct
{
	t_list* pasos_receta;
}t_rta_obt_rec;

#define RTA_OBTENER_RECETA(ptr) ((t_rta_obt_rec*)(ptr))

t_rta_obt_rec* cs_rta_obtener_receta_create(char* pasos, char* tiempos);

#endif /* UTILS_MSG_H_ */
