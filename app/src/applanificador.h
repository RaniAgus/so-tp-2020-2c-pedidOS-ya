#ifndef APPLANIFICADOR_H_
#define APPLANIFICADOR_H_

#include "appconectados.h"
#include "apprepartidores.h"
#include "appenvio.h"

typedef struct{
	uint32_t id_pedido;
	t_repartidor* repartidor;
	t_pos posicionRepartidor;
	char* cliente;
	char* restaurante;
} t_pcb;

void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id);
void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id);

#endif /* APPLANIFICADOR_H_ */

t_list* PCBs;
