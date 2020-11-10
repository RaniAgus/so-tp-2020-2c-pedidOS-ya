#include "applanificador.h"

void app_iniciar_planificador(){
	PCBs = list_create();
}

void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id){
    CS_LOG_TRACE(
        "Se va a crear el PCB: {CLIENTE: %s} {RESTAURANTE: %s} {ID_PEDIDO: %d}", 
        cliente, restaurante, pedido_id
    );

    t_pcb pcb;
    pcb->id_pedido = pedido_id;
    pcb->repartidor = app_asignar_repartidor();
    pcb->posicionRepartidor.x = pcb->repartidor->posicion.x;
    pcb->posicionRepartidor.y = pcb->repartidor->posicion.y;
    pcb->cliente = cliente;
    pcb->restaurante = restaurante;
    list_add(PCBs, pcb);
}

t_repartidor* app_asignar_repartidor(t_pcb* pcb){

	//TODO: [APP] Ver como filtrar bloqueado xq esta descansando o xq esta disponible

	bool esta_libre(void* elemento){	//Funcion para determinar si esta nuevo o bloqueado
		t_repartidor* repartidor = malloc(sizeof(t_repartidor));
		repartidor = elemento;
		if(repartidor->estado == NUEVO || repartidor->estado == BLOQUEADO){
			return true;
		}
		return false;
	}

	t_list* repartidores_libres = list_filter(Repartidores, esta_libre); //Filtro la lista de repartidores x los que esten en nuevo o bloqueado

	float calcular_norma(t_pos vector){
		int a = pow(vector.x, 2);
		int b = pow(vector.y, 2);
		float norma = sqrt(a + b);
		return norma;
	}

	t_pos* calcular_vector_distancia(t_pos posicion1, t_pos posicion2){
		t_pos* distancia;
		distancia->x = posicion1.x - posicion2.x;
		distancia->y = posicion1.y - posicion2.y;
		return distancia;
	}

	app_restaurante_t* restaurante_pcb = app_obtener_copia_restaurante_conectado(pcb->restaurante);

	bool menor_distancia (void* elemento1, void* elemento2){
		t_repartidor* repartidor1 = malloc(sizeof(t_repartidor));
		repartidor1 = elemento1;
		t_repartidor* repartidor2 = malloc(sizeof(t_repartidor));
		repartidor2 = elemento2;
		t_pos vector_distancia1 = caclular_vector_distancia(repartidor1->posicion, restaurante_pcb->posicion);
		float distancia1 = calcular_norma(vector_distancia1);
		t_pos vector_distancia2 = caclular_vector_distancia(repartidor2->posicion, restaurante_pcb->posicion);
		float distancia2 = calcular_norma(vector_distancia2);
		if (distancia1 < distancia2){
			return true;
		}
		return false;
	}

	list_sort(repartidores_libres, menor_distancia); //Ordeno la lista por menor distancia
	t_repartidor* repartidor = list_get(repartidores_libres, 0);	//Me quedo con el head de la lista y asi obtengo al repartidor mas cercano disponible

	return repartidor;
}

void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id)
{
    CS_LOG_TRACE(
        "Se va a avisar al repartidor correspondiente que el pedido está terminado: {RESTAURANTE: %s} {ID_PEDIDO: %d}",
		restaurante, pedido_id
    );

    //TODO: [APP] Avisar pedido terminado, y quitar el "app_finalizar_pedido"
    app_finalizar_pedido(restaurante, pedido_id, "Cliente1");
}
