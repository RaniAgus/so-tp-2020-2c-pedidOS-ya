#include "applanificador.h"

void ap_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id)
{
    CS_LOG_TRACE(
        "Se va a crear el PCB: {CLIENTE: %s} {RESTAURANTE: %s} {ID_PEDIDO: %d}", 
        cliente, restaurante, pedido_id
    );

    //TODO: [APP] Crear PCB
}

void ap_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id)
{
    CS_LOG_TRACE(
        "Se va a avisar al repartidor correspondiente que el pedido está terminado: {RESTAURANTE: %s} {ID_PEDIDO: %d}",
		restaurante, pedido_id
    );

    //TODO: [APP] Avisar pedido terminado, y quitar finalizar pedido
    ap_finalizar_pedido(restaurante, pedido_id, "Cliente1");
}
