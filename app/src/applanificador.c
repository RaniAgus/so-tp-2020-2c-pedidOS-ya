#include "applanificador.h"

void ap_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id)
{
    CS_LOG_TRACE(
        "Se va a crear el PCB: {CLIENTE: %s} {RESTAURANTE: %s} {ID_PEDIDO: %s}", 
        cliente, restaurante, pedido_id
    );
}

void ap_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id)
{
    CS_LOG_TRACE(
        "Se va a avisar al repartidor correspondiente que el pedido está terminado: {RESTAURANTE: %s} {ID_PEDIDO: %s}",
		restaurante, pedido_id
    );
}
