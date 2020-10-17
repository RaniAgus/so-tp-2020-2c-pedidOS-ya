#ifndef RESTPLANIFICADOR_H_
#define RESTPLANIFICADOR_H_

#include "restcore.h"
#include "restsend.h"

void rest_generar_pcbs(uint32_t pedido_id, t_rta_obt_ped* pedido, char* cliente);

#endif /* RESTPLANIFICADOR_H_ */
