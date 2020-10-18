#ifndef RESTENVIO_H_
#define RESTENVIO_H_

#include "restcore.h"

t_rta_obt_rest* rest_obtener_metadata(void);
void 			rest_app_connect(void);

void* 			rest_consultar_sindicato(int8_t msg_type, t_consulta* consulta, int8_t* result);

t_rta_obt_ped*  rest_obtener_pedido(uint32_t pedido_id, int8_t* result);
t_rta_obt_rec*  rest_obtener_receta(char* comida, int8_t* result);

int8_t 			rest_plato_listo(t_sfd conn, pthread_mutex_t* mutex_conn, char* comida, uint32_t pedido_id);
void 			rest_terminar_pedido(uint32_t pedido_id, int8_t* result);


#endif /* RESTENVIO_H_ */
