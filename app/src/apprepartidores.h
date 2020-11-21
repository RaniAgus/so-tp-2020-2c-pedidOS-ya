#ifndef APPREPARTIDORES_H_
#define APPREPARTIDORES_H_

#include "appqueues.h"

typedef struct
{
	sem_t inicio_ejecucion;
	sem_t fin_ejecucion;
	sem_t inicio_derivacion;
	sem_t fin_derivacion;
	sem_t inicio_extraccion;
	sem_t fin_extraccion;
}app_ciclo_t;

void app_iniciar_repartidores(void);
void app_iniciar_ciclo_cpu(void);
void app_esperar_fin_ciclo_cpu(void);

#endif /* APPREPARTIDORES_H_ */
