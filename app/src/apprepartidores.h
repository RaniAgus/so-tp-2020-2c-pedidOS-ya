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

int GRADO_DE_MULTIPROCESAMIENTO;

app_ciclo_t**  	array_sem_ciclo_cpu;

pthread_t* 		hilos_procesadores;


void loggear_movimiento(t_repartidor* repartidor, t_pos anterior, t_pos destino);
void mover_x_repartidor(t_repartidor* repartidor, t_pos destino);
void mover_y_repartidor(t_repartidor* repartidor, t_pos destino);
bool app_mover_repartidor(t_repartidor* repartidor, bool alternador);

void app_iniciar_repartidores(void);
void app_iniciar_ciclo_cpu(void);
void app_esperar_fin_ciclo_cpu(void);
void app_rutina_procesador(app_ciclo_t*  array_sem_ciclo_cpu);

#endif /* APPREPARTIDORES_H_ */
