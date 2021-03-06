#ifndef RESTPLANIFICADOR_H_
#define RESTPLANIFICADOR_H_

#include "restcore.h"
#include "restenvio.h"
#include "restclientes.h"
#include "restdispatcher.h"

void rest_planificador_init(uint32_t pedidos);

uint32_t rest_generar_id(void);
int  	 rest_planificar_plato(char* comida, uint32_t pedido_id, t_list* pasos_receta, char* cliente);

#endif /* RESTPLANIFICADOR_H_ */
