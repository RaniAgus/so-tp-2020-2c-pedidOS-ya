#ifndef APPLANIFICADOR_H_
#define APPLANIFICADOR_H_

#include "appconectados.h"
#include "apprepartidores.h"

typedef enum{
	FIFO,
	HRRN,
	SJFSD,
}e_algoritmo;

void app_iniciar_planificador(void);
void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id);

e_algoritmo algoritmo_planificacion;
sem_t *pcb_sem;

#endif /* APPLANIFICADOR_H_ */
