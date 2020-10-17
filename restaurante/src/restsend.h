#ifndef RESTSEND_H_
#define RESTSEND_H_

#include "restcore.h"

t_rta_obt_rest* rest_obtener_metadata(void);
void 			rest_enviar_handshake(void);
t_rta_obt_ped*  rest_obtener_pedido(uint32_t pedido_id, int8_t* result);
void* 			rest_consultar_sindicato(int8_t msg_type, t_consulta* consulta, int8_t* result);

#endif /* RESTSEND_H_ */
