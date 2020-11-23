#include "applanificador.h"

void app_iniciar_planificador_largo_plazo(void)
{
	pcbs_nuevos = queue_create();
	pthread_mutex_init(&pcbs_nuevos_mutex, NULL);
	sem_init(&pcbs_nuevos_sem, 0, 0);

	//Creo el hilo que se encarga de asignar los repartidores a cada pcb nuevo
	pthread_create(&thread_planificador, NULL, (void*) app_asignar_repartidor, NULL);
	pthread_detach(thread_planificador);
}

//Al confirmar pedido, se crea el PCB correspondiente y se agrega a la lista de pcbs nuevos
void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id)
{
    CS_LOG_TRACE(
        "Se va a crear el PCB: {CLIENTE: %s} {RESTAURANTE: %s} {ID_PEDIDO: %d}", 
        cliente, restaurante, pedido_id
    );

    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->id_pedido = pedido_id;
    pcb->cliente = string_duplicate(cliente);
    pcb->posicionCliente = app_posicion_cliente(cliente);
    pcb->restaurante = string_duplicate(restaurante);
    pcb->posicionRestaurante = app_posicion_restaurante(restaurante);
    pcb->estimacion_anterior = cs_config_get_double("ESTIMACION_INICIAL");
    pcb->ultima_rafaga = cs_config_get_double("ESTIMACION_INICIAL");
    pcb->espera = 0;

    //Arranca sin repartidor asignado, otro hilo lo extrae de la cola y le asigna uno
    queue_sync_push(pcbs_nuevos, &pcbs_nuevos_mutex, &pcbs_nuevos_sem, pcb);
}

//********* FUNCIONES PRIVADAS

void app_asignar_repartidor(t_pcb* pcb)
{
	while(true)
	{
		//Extrae pcb sin asignar
		t_pcb* pcb = queue_sync_pop(pcbs_nuevos, &pcbs_nuevos_mutex, &pcbs_nuevos_sem);

		CS_LOG_DEBUG("Se extrajo el PCB: {CLIENTE: %s} {RESTAURANTE: %s} {POS_DESTINO: [%d,%d]} {ID_PEDIDO: %d}"
				, pcb->cliente
				, pcb->restaurante
				, pcb->posicionRestaurante.x
				, pcb->posicionRestaurante.y
				, pcb->id_pedido
		);

		//Extrae repartidor libre
		t_repartidor* repartidor = app_obtener_repartidor_libre(pcb->posicionRestaurante);

		//Asigna el PCB al repartidor
		repartidor->pcb = pcb;
		repartidor->destino = DESTINO_RESTAURANTE;

		CS_LOG_DEBUG("Se asignó el repartidor: {ID: %d} {POS_REPARTIDOR: [%d,%d]} {POS_DESTINO: [%d,%d]}"
				, repartidor->id
				, repartidor->posicion.x
				, repartidor->posicion.y
				, repartidor->pcb->posicionRestaurante.x
				, repartidor->pcb->posicionRestaurante.y
		);

		//Deriva el repartidor a la queue correspondiente (ready/bloqueado/etc)
		app_derivar_repartidor(repartidor);
	}
}

