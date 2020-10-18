#include "restplanifcortoplazo.h"

static sem_t** array_sem_inicio_ciclo_cpu;
static sem_t** array_sem_fin_ciclo_cpu;

static t_queue* 	   queue_entrada_salida;
static pthread_mutex_t mutex_entrada_salida;

static int rest_agregar_planificador_corto_plazo(sem_t** sem_inicio_ciclo_cpu, sem_t** sem_fin_ciclo_cpu);

void rest_planificador_corto_plazo_init(void)
{
	array_sem_inicio_ciclo_cpu = (sem_t**)string_array_new();
	array_sem_fin_ciclo_cpu = (sem_t**)string_array_new();

	queue_entrada_salida = queue_create();
	pthread_mutex_init(&mutex_entrada_salida, NULL);
}

void rest_iniciar_ciclo_cpu(void)
{
	void _hacer_signal_a_todos(sem_t* semaforo) {
		sem_post(semaforo);
	}
	string_iterate_lines((char**)array_sem_inicio_ciclo_cpu, (void*) _hacer_signal_a_todos);
}

void rest_esperar_fin_ciclo_cpu(void)
{
	void _hacer_signal_a_todos(sem_t* semaforo) {
		sem_wait(semaforo);
	}
	string_iterate_lines((char**)array_sem_fin_ciclo_cpu, (void*) _hacer_signal_a_todos);
}

//TODO: [RESTAURANTE] Inicializar cocinero
void rest_cocinero_routine(rest_cola_ready_t* queue_ready)
{
	sem_t *sem_inicio_ciclo_cpu, *sem_fin_ciclo_cpu;
	int planificador_id = rest_agregar_planificador_corto_plazo(&sem_inicio_ciclo_cpu, &sem_fin_ciclo_cpu);
	CS_LOG_TRACE("Se creó un cocinero (planificador_id = %d).", planificador_id);

	while(1)
	{
		sem_wait(sem_inicio_ciclo_cpu);

		/*
		 * <Acciones de cocinero>
		 *
		 */

		sem_post(sem_fin_ciclo_cpu);
	}
}

//TODO: [RESTAURANTE] Inicializar horno
void rest_horno_routine()
{
	sem_t *sem_inicio_ciclo_cpu, *sem_fin_ciclo_cpu;
	int planificador_id = rest_agregar_planificador_corto_plazo(&sem_inicio_ciclo_cpu, &sem_fin_ciclo_cpu);
	CS_LOG_TRACE("Se creó un horno (planificador_id = %d).", planificador_id);

	while(1)
	{
		sem_wait(sem_inicio_ciclo_cpu);

		/*
		 * <Acciones de horno>
		 *
		 */

		sem_post(sem_fin_ciclo_cpu);
	}
}

static int rest_agregar_planificador_corto_plazo(sem_t** sem_inicio_ciclo_cpu, sem_t** sem_fin_ciclo_cpu)
{
	*sem_inicio_ciclo_cpu = malloc(sizeof(sem_t));
	*sem_fin_ciclo_cpu = malloc(sizeof(sem_t));
	sem_init(*sem_inicio_ciclo_cpu, 0, 0);
	sem_init(*sem_fin_ciclo_cpu, 0, 0);

	string_array_push((char***)&array_sem_inicio_ciclo_cpu, (char*) *sem_inicio_ciclo_cpu);
	string_array_push((char***)&array_sem_fin_ciclo_cpu, (char*) *sem_fin_ciclo_cpu);

	return string_array_size((char**)array_sem_inicio_ciclo_cpu) - 1;
}
