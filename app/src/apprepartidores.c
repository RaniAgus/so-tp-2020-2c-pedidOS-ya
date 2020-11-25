#include "apprepartidores.h"

static pthread_t*      hilos_procesadores;

static app_ciclo_t**   array_sem_ciclo_cpu;
static int             GRADO_DE_MULTIPROCESAMIENTO;

static t_list*         repartidores_descansando;
static pthread_mutex_t repartidores_descansando_mutex;

static void app_rutina_procesador(app_ciclo_t*  array_sem_ciclo_cpu);

static void loggear_movimiento(t_repartidor* repartidor, t_pos anterior, t_pos destino);
static void mover_x_repartidor(t_repartidor* repartidor, t_pos destino);
static void mover_y_repartidor(t_repartidor* repartidor, t_pos destino);
static bool app_mover_repartidor(t_repartidor* repartidor, bool alternador);

static bool toca_descansar(t_repartidor* repartidor);
static void app_agregar_repartidor_descansando(t_repartidor* repartidor);
static void app_reviso_repartidores_descansados(void);

void app_iniciar_repartidores(void)
{
	GRADO_DE_MULTIPROCESAMIENTO = cs_config_get_int("GRADO_DE_MULTIPROCESAMIENTO");

	repartidores_descansando = list_create();
	pthread_mutex_init(&repartidores_descansando_mutex, NULL);

	char** paresDeCoordenadas = cs_config_get_array_value("REPARTIDORES");
	char** frecuenciasDeDescanso = cs_config_get_array_value("FRECUENCIA_DE_DESCANSO");
	char** tiemposDeDescanso = cs_config_get_array_value("TIEMPO_DE_DESCANSO");

	for(int i = 0; paresDeCoordenadas[i] != NULL; i++)
	{
		char** posicion = string_split(paresDeCoordenadas[i], "|");
		t_repartidor* repartidor = malloc(sizeof(t_repartidor));

		repartidor->pcb = NULL;
		repartidor->id = i+1;
		repartidor->posicion.x = atoi(posicion[0]);
		repartidor->posicion.y = atoi(posicion[1]);
		repartidor->ciclos_sin_descansar = 0;
		repartidor->frecuencia_de_descanso = atoi(frecuenciasDeDescanso[i]);
		repartidor->tiempo_de_descanso = atoi(tiemposDeDescanso[i]);

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

	array_sem_ciclo_cpu = (app_ciclo_t**)string_array_new();
	hilos_procesadores = calloc(GRADO_DE_MULTIPROCESAMIENTO, sizeof(pthread_t));
	for(int i = 0; i < GRADO_DE_MULTIPROCESAMIENTO; i++)
	{
		app_ciclo_t* semaforos = malloc(sizeof(app_ciclo_t));
		sem_init(&semaforos->inicio_ejecucion, 0, 0);
		sem_init(&semaforos->fin_ejecucion, 0, 0);
		sem_init(&semaforos->inicio_derivacion, 0, 0);
		sem_init(&semaforos->fin_derivacion, 0, 0);
		sem_init(&semaforos->inicio_extraccion, 0, 0);
		sem_init(&semaforos->fin_extraccion, 0, 0);

		string_array_push((void*)&array_sem_ciclo_cpu, (void*)semaforos);
		pthread_create(&hilos_procesadores[i], NULL, (void*)app_rutina_procesador, semaforos);
		pthread_detach(hilos_procesadores[i]);
	}
}

/*********************************** RUTINA DEL REPARTIDOR ***********************************/

void app_iniciar_ciclo_cpu(void)
{
	//Ejecucion

	void _hacer_signal_ejecucion(app_ciclo_t* semaforo) {
		sem_post(&semaforo->inicio_ejecucion);
	}
	string_iterate_lines((void*)array_sem_ciclo_cpu, (void*) _hacer_signal_ejecucion);

	void descansar(t_repartidor* repartidor) {
		repartidor->ciclos_sin_descansar --;
		CS_LOG_DEBUG("El repartidor está descansando: {REPARTIDOR: %d; DESCANSO: %d/%d}"
				, repartidor->tiempo_de_descanso - repartidor->ciclos_sin_descansar
				, repartidor->tiempo_de_descanso
		);
	}
	list_iterate(repartidores_descansando, (void*)descansar);

	void _hacer_wait_ejecucion(app_ciclo_t* semaforo) {
		sem_wait(&semaforo->fin_ejecucion);
	}
	string_iterate_lines((void*)array_sem_ciclo_cpu, (void*) _hacer_wait_ejecucion);

	//Derivacion

	void _hacer_signal_derivacion(app_ciclo_t* semaforo) {
		sem_post(&semaforo->inicio_derivacion);
	}
	string_iterate_lines((void*)array_sem_ciclo_cpu, (void*) _hacer_signal_derivacion);

	app_reviso_repartidores_descansados();

	void _hacer_wait_derivacion(app_ciclo_t* semaforo) {
		sem_wait(&semaforo->fin_derivacion);
	}
	string_iterate_lines((void*)array_sem_ciclo_cpu, (void*) _hacer_wait_derivacion);

	//Extraccion

	void _hacer_signal_extraccion(app_ciclo_t* semaforo) {
		sem_post(&semaforo->inicio_extraccion);
	}
	string_iterate_lines((void*)array_sem_ciclo_cpu, (void*) _hacer_signal_extraccion);
}

void app_esperar_fin_ciclo_cpu(void)
{
	void _hacer_wait_extraccion(app_ciclo_t* semaforo) {
		sem_wait(&semaforo->fin_extraccion);
	}
	string_iterate_lines((void*)array_sem_ciclo_cpu, (void*) _hacer_wait_extraccion);
}

void app_rutina_procesador(app_ciclo_t* semaforo)
{
	t_repartidor* repartidor = NULL;
	bool alternador = true;

	while(true)
	{
		sem_wait(&semaforo->inicio_ejecucion);
		if(repartidor != NULL)
		{
			repartidor->ciclos_sin_descansar++;
			repartidor->pcb->ultima_rafaga++;
			alternador = app_mover_repartidor(repartidor, alternador);
		}
		sem_post(&semaforo->fin_ejecucion);

		sem_wait(&semaforo->inicio_derivacion);
		if(repartidor != NULL)
		{
			if(toca_descansar(repartidor))
			{
				app_agregar_repartidor_descansando(repartidor);
				repartidor = NULL;
			} else if(repartidor_llego_a_destino(repartidor))
			{
				app_derivar_repartidor(repartidor);
				repartidor = NULL;
			}
		}
		sem_post(&semaforo->fin_derivacion);

		sem_wait(&semaforo->inicio_extraccion);
		if(repartidor == NULL)
		{
			repartidor = app_ready_pop();
			if(repartidor) CS_LOG_INFO("El repartidor pasó a EXEC: {REPARTIDOR: %d, POS: [%d,%d]} {PEDIDO: %d; RESTAURANTE: %s}"
					, repartidor->id
					, repartidor->posicion.x
					, repartidor->posicion.y
					, repartidor->pcb->id_pedido
					, repartidor->pcb->restaurante
			);
		}
		sem_post(&semaforo->fin_extraccion);
	}
}

/********************************** POSICION DEL REPARTIDOR **********************************/

bool app_mover_repartidor(t_repartidor* repartidor, bool alternador)
{
	if(alternador) {
		mover_x_repartidor(repartidor, app_destino_repartidor(repartidor));
	} else {
		mover_y_repartidor(repartidor, app_destino_repartidor(repartidor));
	}
	return !alternador;
}

void mover_x_repartidor(t_repartidor* repartidor, t_pos destino)
{
	t_pos anterior = repartidor->posicion;

	if(repartidor->posicion.x < destino.x) {
		repartidor->posicion.x ++;
		loggear_movimiento(repartidor, anterior, destino);
	} else if(repartidor->posicion.x > destino.x) {
		repartidor->posicion.x --;
		loggear_movimiento(repartidor, anterior, destino);
	} else {
		//Si el repartidor ya esta en el x del destino, que mueva la y.
		mover_y_repartidor(repartidor, destino);
	}
}

void mover_y_repartidor(t_repartidor* repartidor, t_pos destino)
{
	t_pos anterior = repartidor->posicion;
	if(repartidor->posicion.y < destino.y) {
		repartidor->posicion.y ++;
		loggear_movimiento(repartidor, anterior, destino);
	} else if(repartidor->posicion.y > destino.y) {
		repartidor->posicion.y --;
		loggear_movimiento(repartidor, anterior, destino);
	} else {
		//Si el repartidor ya esta en el y del destino, que mueva la x.
		mover_x_repartidor(repartidor, destino);
	}
}

void loggear_movimiento(t_repartidor* repartidor, t_pos anterior, t_pos destino)
{
	CS_LOG_INFO("El repartidor se movió: {REPARTIDOR: %d; POS: [%d,%d] -> [%d,%d]; DESTINO: [%d,%d]; CICLOS: %d/%d}"
			, repartidor->id
			, anterior.x
			, anterior.y
			, repartidor->posicion.x
			, repartidor->posicion.y
			, destino.x
			, destino.y
			, repartidor->ciclos_sin_descansar
			, repartidor->frecuencia_de_descanso
	);
}

/********************************** REPARTIDORES BLOQUEADOS POR DESCANSO **********************************/

bool toca_descansar(t_repartidor* repartidor)
{
	return (repartidor->ciclos_sin_descansar == repartidor->frecuencia_de_descanso);
}

void app_agregar_repartidor_descansando(t_repartidor* repartidor)
{
	pthread_mutex_lock(&repartidores_descansando_mutex);
	list_add(repartidores_descansando, repartidor);
	pthread_mutex_unlock(&repartidores_descansando_mutex);

	CS_LOG_INFO("El repartidor pasó a BLOQUEADO para descansar: {REPARTIDOR: %d; POS: [%d,%d]; CICLOS: %d/%d}"
			, repartidor->id
			, repartidor->posicion.x
			, repartidor->posicion.y
			, repartidor->ciclos_sin_descansar
			, repartidor->frecuencia_de_descanso
	);
}

void app_reviso_repartidores_descansados(void)
{
	pthread_mutex_lock(&repartidores_descansando_mutex);

	int i = 0;
	void ya_descanso(t_repartidor* repartidor) {
		if(repartidor->ciclos_sin_descansar == 0)
		{
			CS_LOG_DEBUG("El repartidor terminó de descansar: {REPARTIDOR: %d; DESCANSO: %d/%d} {PEDIDO: %d; RESTAURANTE: %s}"
				, repartidor->tiempo_de_descanso - repartidor->ciclos_sin_descansar
				, repartidor->tiempo_de_descanso
				, repartidor->pcb->id_pedido
				, repartidor->pcb->restaurante
			);
			list_remove(repartidores_descansando, i);
			app_derivar_repartidor(repartidor);
		}
		else i++;
	}
	list_iterate(repartidores_descansando, (void*) ya_descanso);

	pthread_mutex_unlock(&repartidores_descansando_mutex);

}

