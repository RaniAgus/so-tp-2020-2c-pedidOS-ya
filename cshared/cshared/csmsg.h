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
	OBTENER_RECETA,

	HANDSHAKE
}e_msgtype;

#define MSGTYPES_CANT 16+1

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
char* 	cs_msg_to_str(void* msg, int8_t op_code, int8_t msg_type);

/**
* @NAME cs_msg_destroy
* @DESC Destruye un mensaje y su contenido.
*/
void 	cs_msg_destroy(void* msg, int8_t op_code, int8_t msg_type);

//*************************MENSAJES************************

typedef struct
{
	int8_t msgtype;

	char* 	 comida;
	uint32_t cantidad;
	char* 	 restaurante;
	uint32_t pedido_id;
}t_consulta;

#define CONSULTA_PTR(ptr) ((t_consulta*)(ptr))

typedef enum
{
	CONS_ARG_COMIDA = 0,
	CONS_ARG_CANTIDAD,
	CONS_ARG_RESTAURANTE,
	CONS_ARG_PEDIDO_ID
}e_cons_arg;

#define CONS_ARGS_CANT 4

/**
* @NAME cs_cons_has_argument
* @DESC Devuelve un boolean indicando si el tipo de consulta
* tiene ese argumento.
*/
bool cs_cons_has_argument(int8_t msgtype, int8_t arg, int8_t module);

t_consulta* _cons_create(int8_t msgtype, char* plato, uint32_t cant, char* rest, uint32_t pedido_id);

/**
* @NAME cs_cons/rta_create
* @DESC Crean una estructura consulta o respuesta.
*/

//*****************************HANDSHAKE**********************************

typedef struct
{
	char*	nombre;
	t_pos 	posicion;
}t_handshake;

#define HANDSHAKE_PTR(ptr) ((t_handshake*)(ptr))

t_handshake* cs_cons_handshake_create(char* nombre, uint32_t posx, uint32_t posy);

typedef enum
{
	MODULO_DESCONOCIDO = -3,
	MODULO_COMANDA = -2,
	MODULO_SINDICATO = -1,
	MODULO_CLIENTE = 0,
	MODULO_APP = 1,
	MODULO_RESTAURANTE = 2
}e_module;

const char* cs_enum_module_to_str(int value);

typedef struct
{
	int8_t 	modulo;
}t_rta_handshake;

#define RTA_HANDSHAKE_PTR(ptr) ((t_rta_handshake*)(ptr))

t_rta_handshake* cs_rta_handshake_create(void);

//*************************CONSULTAR RESTAURANTES*************************

#define cs_msg_consultar_rest_create()\
	_cons_create((int8_t)CONSULTAR_RESTAURANTES, NULL, 0, NULL, 0)

typedef struct
{
	char** restaurantes;
}t_rta_cons_rest;

#define RTA_CONS_REST(ptr) ((t_rta_cons_rest*)(ptr))

t_rta_cons_rest* cs_rta_consultar_rest_create(char* restaurantes);

//*************************SELECCIONAR RESTAURANTE*************************

#define cs_msg_seleccionar_rest_create(rest)\
		_cons_create((int8_t)SELECCIONAR_RESTAURANTE, NULL, 0, rest, 0)

//*************************OBTENER RESTAURANTE*************************

#define cs_msg_obtener_rest_create(rest)\
		_cons_create((int8_t)OBTENER_RESTAURANTE, NULL, 0, rest, 0)

typedef struct
{
	uint32_t cant_cocineros;
	char** 	 afinidades;
	t_list*  menu;
	t_pos 	 pos_restaurante;
	uint32_t cant_hornos;
	uint32_t cant_pedidos;
}t_rta_obt_rest;

#define RTA_OBT_REST(ptr) ((t_rta_obt_rest*)(ptr))

t_rta_obt_rest* cs_rta_obtener_rest_create(uint32_t cant_cocineros,
								   	   	   char* 	afinidades,
										   char*	comidas,
										   char*	precios,
										   t_pos 	pos_restaurante,
										   uint32_t cant_hornos,
										   uint32_t cant_pedidos);

//*************************CONSULTAR PLATOS*************************

#define cs_msg_consultar_pl_create()\
		_cons_create((int8_t)CONSULTAR_PLATOS, NULL, 0, NULL, 0)

#define cs_msg_consultar_pl_rest_create(rest)\
		_cons_create((int8_t)CONSULTAR_PLATOS, NULL, 0, rest, 0)

typedef struct
{
	char** comidas;
}t_rta_cons_pl;

#define RTA_CONS_PL(ptr) ((t_rta_cons_pl*)(ptr))

t_rta_cons_pl* cs_rta_consultar_pl_create(char* platos);

//*************************CREAR PEDIDO*************************

#define cs_msg_crear_ped_create()\
		_cons_create((int8_t)CREAR_PEDIDO, NULL, 0, NULL, 0)

typedef struct
{
	uint32_t pedido_id;
}t_rta_crear_ped;

#define RTA_CREAR_PED(ptr) ((t_rta_crear_ped*)(ptr))

t_rta_crear_ped* cs_rta_crear_ped_create(uint32_t pedido_id);

//*************************GUARDAR PEDIDO*************************

#define cs_msg_guardar_ped_create(rest, pedido_id)\
		_cons_create((int8_t)GUARDAR_PEDIDO, NULL, 0, rest, pedido_id)

//*************************AÑADIR PLATO*************************

#define cs_msg_aniadir_pl_create(plato, pedido_id)\
		_cons_create((int8_t)ANIADIR_PLATO, plato, 0, NULL, pedido_id)

//*************************GUARDAR PLATO*************************

#define cs_msg_guardar_pl_create(plato, cant, rest, pedido_id)\
		_cons_create((int8_t)GUARDAR_PLATO, plato, cant, rest, pedido_id)

//*************************CONFIRMAR PEDIDO*************************

#define cs_msg_confirmar_ped_create(pedido_id)\
		_cons_create((int8_t)CONFIRMAR_PEDIDO, NULL, 0, NULL, pedido_id)

#define cs_msg_confirmar_ped_rest_create(rest, pedido_id)\
		_cons_create((int8_t)CONFIRMAR_PEDIDO, NULL, 0, rest, pedido_id)

//*************************PLATO LISTO*************************

#define cs_msg_plato_listo_create(plato, rest, pedido_id)\
		_cons_create((int8_t)PLATO_LISTO, plato, 0, rest, pedido_id)

//*************************CONSULTAR PEDIDO*************************

#define cs_msg_consultar_ped_create(pedido_id)\
		_cons_create((int8_t)CONSULTAR_PEDIDO, NULL, 0, NULL, pedido_id)

typedef struct
{
	char* 	restaurante;
	int8_t 	estado_pedido;
	t_list*	platos_y_estados;
}t_rta_cons_ped;

#define RTA_CONSULTAR_PED(ptr)	((t_rta_cons_ped*)(ptr))

t_rta_cons_ped* cs_rta_consultar_ped_create(char* rest,
											e_estado_ped estado_ped,
											char* platos,
											char* listos,
											char* totales);

//*************************OBTENER PEDIDO*************************

#define cs_msg_obtener_ped_create(rest, pedido_id)\
		_cons_create((int8_t)OBTENER_PEDIDO, NULL, 0, rest, pedido_id)

typedef struct
{
	int8_t 	estado_pedido;
	t_list*	platos_y_estados;
}t_rta_obt_ped;

#define RTA_OBTENER_PED(ptr)	((t_rta_obt_ped*)(ptr))

t_rta_obt_ped* cs_rta_obtener_ped_create(e_estado_ped estado_ped, char* platos, char* listos, char* totales);

//*************************FINALIZAR PEDIDO*************************

#define cs_msg_fin_ped_create(rest, pedido_id)\
		_cons_create((int8_t)FINALIZAR_PEDIDO, NULL, 0, rest, pedido_id)

//*************************TERMINAR PEDIDO*************************

#define cs_msg_term_ped_create(rest, pedido_id)\
		_cons_create((int8_t)TERMINAR_PEDIDO, NULL, 0, rest, pedido_id)

//*************************OBTENER RECETA*************************

#define cs_msg_rta_obtener_receta_create(plato)\
		_cons_create((int8_t)OBTENER_RECETA, plato, 0, NULL, 0)

typedef struct
{
	t_list* pasos_receta;
}t_rta_obt_rec;

#define RTA_OBTENER_RECETA(ptr) ((t_rta_obt_rec*)(ptr))

t_rta_obt_rec* cs_rta_obtener_receta_create(char* pasos, char* tiempos);

#endif /* UTILS_MSG_H_ */
