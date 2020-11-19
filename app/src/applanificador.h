#ifndef APPLANIFICADOR_H_
#define APPLANIFICADOR_H_

#include "appqueues.h"

void app_iniciar_planificador_largo_plazo(void);
void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id);

#endif /* APPLANIFICADOR_H_ */
