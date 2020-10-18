#ifndef RESTPLANIFICADOR_H_
#define RESTPLANIFICADOR_H_

#include "restcore.h"
#include "restclientes.h"
#include "restenvio.h"

typedef enum
{
	ESTADO_NEW = 1,
	ESTADO_READY,
	ESTADO_BLOCK,
	ESTADO_EXEC,
	ESTADO_EXIT
}rest_estado_e;

typedef struct
{
	rest_estado_e estado;
	t_list* pasos_restantes;

	char* comida;
	uint32_t pedido_id;

	t_sfd conexion;
	pthread_mutex_t* mutex_conexion;
}rest_pcb_t;

typedef struct
{
	t_list*			comidas;
	t_queue* 		queue;
	pthread_mutex_t mutex_queue;
	sem_t 			sem_queue;
}rest_cola_ready_t;

void rest_planificador_init(t_rta_obt_rest* metadata);

uint32_t rest_generar_id(void);
void rest_planificar_plato(char* comida, uint32_t pedido_id, t_list* pasos_receta, char* cliente);

#endif /* RESTPLANIFICADOR_H_ */
