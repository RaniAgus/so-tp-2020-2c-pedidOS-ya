#ifndef APSEND_H_
#define APSEND_H_

#include "apconn.h"

void   ap_plato_listo(t_consulta* consulta, int8_t* result);
int8_t ap_finalizar_pedido(char* restaurante, uint32_t pedido_id, char* cliente);

t_rta_obt_ped* ap_obtener_pedido(char* restaurante, uint32_t pedido_id, int8_t* result);
void		   ap_guardar_pedido(char* restaurante, uint32_t pedido_id, int8_t* result);
void		   ap_guardar_plato(char* comida, char* restaurante, uint32_t pedido_id, int8_t* result);
void		   ap_confirmar_pedido(char* restaurante, uint32_t pedido_id, int8_t* result);

void* ap_consultar_restaurante(char* ip, char* puerto, int8_t msg_type, t_consulta* consulta, int8_t* result);


#endif /* APSEND_H_ */
