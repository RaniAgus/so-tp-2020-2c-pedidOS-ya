#include "apprepartidores.h"

static pthread_t* threads_dispatcher;

static void app_dispatcher_routine(void);

void app_iniciar_repartidores(void)
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
		repartidor->ciclos_sin_descansar = 0;
		repartidor->frecuencia_de_descanso = atoi(frecuenciasDeDescanso[i]);
		repartidor->tiempo_de_descanso = atoi(tiemposDeDescanso[i]);
		repartidor->estimacion_anterior = cs_config_get_double("ESTIMACION_INICIAL");
		repartidor->ultima_rafaga = cs_config_get_double("ESTIMACION_INICIAL");
		repartidor->espera = 0;

		//Al inicio se agregan todos los repartidores creados a la lista de libres
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

	int gm = cs_config_get_int("GRADO_DE_MULTIPROCESAMIENTO");
	threads_dispatcher = calloc(gm, sizeof(pthread_t));

	//TODO: Ver si crear un hilo por procesador o por repartidor
	for(int i = 0; i < gm; i++) {
		pthread_create(&threads_dispatcher[i], NULL, (void*) app_dispatcher_routine, NULL);
		pthread_detach(threads_dispatcher[i]);
	}
}

static t_pos pos_destino(t_repartidor* repartidor)
{
	return repartidor->pcb->destino == DESTINO_RESTAURANTE ? repartidor->pcb->posicionRestaurante : repartidor->pcb->posicionCliente;
}

static void app_dispatcher_routine(void)
{
	t_repartidor* repartidor = NULL;

	while(true)
	{
		//TODO: Semáforos

		if(repartidor != NULL) {
			//TODO: Mover
			repartidor->ciclos_sin_descansar++;
			repartidor->ultima_rafaga++;
		}

		//TODO: Semáforos

		if(repartidor != NULL) {
			if(repartidor->ciclos_sin_descansar == repartidor->frecuencia_de_descanso) {
				//TODO: Bloquear
			} else if(calcular_norma(calcular_vector_distancia(repartidor->posicion, pos_destino(repartidor))) == 0) {
				//TODO: Derivar
			}
		}

		//TODO: Semáforos

		if(repartidor == NULL) {
			repartidor = app_ready_pop();
		}

		//TODO: Semáforos
	}
}
