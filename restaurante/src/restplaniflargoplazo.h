#ifndef RESTPLANIFLARGOPLAZO_H_
#define RESTPLANIFLARGOPLAZO_H_

#include "restcore.h"
#include "restenvio.h"
#include "restclientes.h"
#include "restplanifcortoplazo.h"

void rest_planificador_largo_plazo_init(t_rta_obt_rest* metadata);

uint32_t rest_generar_id(void);
int  	 rest_planificar_plato(char* comida, uint32_t pedido_id, t_list* pasos_receta, char* cliente);

void 		rest_cola_ready_push(rest_cola_ready_t* queue, rest_pcb_t* pcb);
rest_pcb_t* rest_cola_ready_pop(rest_cola_ready_t* queue);

#endif /* RESTPLANIFLARGOPLAZO_H_ */
