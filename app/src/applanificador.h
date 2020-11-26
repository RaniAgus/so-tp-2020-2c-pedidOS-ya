#ifndef APPLANIFICADOR_H_
#define APPLANIFICADOR_H_

#include "appqueues.h"

t_queue*		pcbs_nuevos;

pthread_t 		thread_planificador;
pthread_mutex_t pcbs_nuevos_mutex;

sem_t 			pcbs_nuevos_sem;

void app_asignar_repartidor(t_pcb* pcb);

void app_iniciar_planificador_largo_plazo(void);
void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id);

#endif /* APPLANIFICADOR_H_ */
