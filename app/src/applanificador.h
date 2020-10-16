#ifndef APPLANIFICADOR_H_
#define APPLANIFICADOR_H_

#include "apconn.h"
#include "apsend.h"

void ap_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id);
void ap_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id);

#endif /* APPLANIFICADOR_H_ */
