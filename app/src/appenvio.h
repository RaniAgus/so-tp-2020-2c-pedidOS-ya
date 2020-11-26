#ifndef APPENVIO_H_
#define APPENVIO_H_

#include "appconectados.h"

void   app_plato_listo(t_consulta* consulta, int8_t* result);
int8_t app_finalizar_pedido(char* restaurante, uint32_t pedido_id, char* cliente);

t_rta_obt_ped* app_obtener_pedido(char* restaurante, uint32_t pedido_id, int8_t* result);
void           app_guardar_pedido(char* restaurante, uint32_t pedido_id, int8_t* result);
void           app_guardar_plato(char* comida, char* restaurante, uint32_t pedido_id, int8_t* result);
void           app_confirmar_pedido(char* restaurante, uint32_t pedido_id, int8_t* result);

void* app_consultar_restaurante(char* restaurante, int8_t msg_type, t_consulta* consulta, int8_t* result);

#endif /* APPENVIO_H_ */
