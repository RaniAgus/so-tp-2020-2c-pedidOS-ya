#ifndef APPLANIFICADOR_H_
#define APPLANIFICADOR_H_

#include "appconectados.h"
#include "apprepartidores.h"

void app_iniciar_planificador(void);
void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id);

#endif /* APPLANIFICADOR_H_ */
