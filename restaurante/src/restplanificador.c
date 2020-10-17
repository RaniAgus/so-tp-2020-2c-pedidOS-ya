#include "restplanificador.h"

void rest_generar_pcbs(uint32_t pedido_id, t_rta_obt_ped* pedido, char* cliente)
{
	//TODO: Generar PCBs
	CS_LOG_TRACE("Se generarán los PCBs del pedido %d generado por %s", pedido_id, ({ cliente ? cliente : "App"; }));
	//TODO: Probar Obtener Receta por cada comida
}
