#ifndef RESTDISPATCHER_H_
#define RESTDISPATCHER_H_

#include "restcore.h"

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

void rest_planificador_corto_plazo_init(void);
void rest_cocinero_routine(rest_cola_ready_t* cola_ready);
void rest_horno_routine();

void rest_iniciar_ciclo_cpu(void);
void rest_esperar_fin_ciclo_cpu(void);

#endif /* RESTDISPATCHER_H_ */
