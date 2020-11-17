#include "apprepartidores.h"

//TODO: [APP] Crear e iniciar las listas de ready, bloqueado por descanso y bloqueado por espera

static void app_agregar_repartidor_libre(t_repartidor* repartidor);

void app_inicializar_repartidores(void)
{
	char** paresDeCoordenadas = cs_config_get_array_value("REPARTIDORES");
	char** frecuenciasDeDescanso = cs_config_get_array_value("FRECUENCIA_DE_DESCANSO");
	char** tiemposDeDescanso = cs_config_get_array_value("TIEMPO_DE_DESCANSO");

	for(int i = 0; paresDeCoordenadas[i] != NULL; i++)
	{
		char** posicion = string_split(paresDeCoordenadas[i], "|");
		t_repartidor* repartidor = malloc(sizeof(t_repartidor));

		repartidor->id = i+1;
		repartidor->posicion.x = atoi(posicion[0]);
		repartidor->posicion.y = atoi(posicion[1]);
		repartidor->frecuencia_de_descanso = atoi(frecuenciasDeDescanso[i]);
		repartidor->tiempo_de_descanso = atoi(tiemposDeDescanso[i]);

		app_agregar_repartidor_libre(repartidor);
		string_iterate_lines(posicion, (void*) free);
		free(posicion);
	}

	string_iterate_lines(paresDeCoordenadas, (void*) free);
	string_iterate_lines(frecuenciasDeDescanso, (void*) free);
	string_iterate_lines(tiemposDeDescanso, (void*) free);
	free(paresDeCoordenadas);
	free(frecuenciasDeDescanso);
	free(tiemposDeDescanso);
}

//Luego de recibir Plato Listo, si el pedido está terminado se avisa al repartidor
void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id)
{
    CS_LOG_TRACE(
        "Se va a avisar al repartidor correspondiente que el pedido está terminado: {RESTAURANTE: %s} {ID_PEDIDO: %d}",
		restaurante, pedido_id
    );

    //TODO: [APP] Buscar el PCB en la lista de bloqueados por espera, cambiarle el destino a CLIENTE y derivarlo
}

void app_derivar_pcb(t_pcb* pcb)
{
	//TODO: [APP] Derivar según la posición actual y la destino:
	/*
	 * 1. si al repartidor le toca descansar, se va a la lista de bloqueados por descanso
	 * 2. si posición_actual == posicion_destino y el destino es restaurante, obtiene el pedido y...
	 * 	  a. ...si está listo, se dirige hacia el cliente
	 * 	  b. ...si no está listo, se va a la lista de bloqueados por espera
	 * 3. si posición_actual == posicion_destino y el destino es cliente, finaliza el pedido
	 * 4. si no llegó a su posición destino ni le toca descansar, se va a la lista de ready
	 *
	 * */
}

//Al inicio se agregan todos los repartidores creados a la lista de disponibles
static void app_agregar_repartidor_libre(t_repartidor* repartidor)
{
	pthread_mutex_lock(&repartidores_libres_mutex);
	list_add(repartidores_libres, repartidor);
	pthread_mutex_unlock(&repartidores_libres_mutex);

	sem_post(&repartidores_libres_sem);
}

void app_loggear_posicion(t_repartidor* repartidor) {
	CS_LOG_DEBUG(
			"El repartidor id: %d se movio a la posicion [%d,%d]",
			repartidor->id, repartidor->posicion.x, repartidor->posicion.y);
}

void app_mover_x_repartidor(t_repartidor* repartidor, t_pos* destino)
{
	if(repartidor->posicion.x < destino->x){
		repartidor->posicion.x ++;
		app_loggear_posicion(repartidor);
		}
	else if(repartidor->posicion.x > destino->x){
		repartidor->posicion.x --;
		app_loggear_posicion(repartidor);
		}
	else{
		//Si el repartidor ya esta en el x del destino, que mueva la y.
		app_mover_y_repartidor(repartidor, destino);
	}
}

void app_mover_y_repartidor(t_repartidor* repartidor, t_pos* destino)
{
	if(repartidor->posicion.y < destino->y){
		repartidor->posicion.y ++;
		app_loggear_posicion(repartidor);
		}
	else if(repartidor->posicion.y > destino->y){
		repartidor->posicion.y --;
		app_loggear_posicion(repartidor);
	}
	else{
		//Si el repartidor ya esta en el y del destino, que mueva la x.
		app_mover_x_repartidor(repartidor, destino);
	}
}
