#ifndef APPLANIFICADOR_H_
#define APPLANIFICADOR_H_

#include "appconectados.h"
#include "appenvio.h"

void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id);
void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id);

#endif /* APPLANIFICADOR_H_ */
