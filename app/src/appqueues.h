#ifndef APPCORE_H_
#define APPCORE_H_

#include "appconectados.h"
#include "appenvio.h"

typedef enum {
	NUEVO,
	LISTO,
	BLOQUEADO,
	EJECUTANDO,
	FIN
} e_estados;

typedef enum {
	DESTINO_CLIENTE,
	DESTINO_RESTAURANTE
} e_destinos;

typedef struct {
	e_estados estado;
	uint32_t id_pedido;
	char* cliente;
	t_pos posicionCliente;
	char* restaurante;
	t_pos posicionRestaurante;
	double estimacion_anterior;
	double ultima_rafaga;
	double espera;
} t_pcb;

typedef struct {
	uint32_t id;
	t_pcb* pcb;
	t_pos posicion;
	e_destinos destino;
	int ciclos_sin_descansar;
	int frecuencia_de_descanso;
	int tiempo_de_descanso;
} t_repartidor;

t_list*			repartidores_libres;
t_list* 		ready_queue;
t_list*			repartidores_esperando;
t_list*	       	repartidores_descansando;

pthread_mutex_t repartidores_libres_mutex;
pthread_mutex_t	ready_mutex;
pthread_mutex_t	repartidores_esperando_mutex;
pthread_mutex_t repartidores_descansando_mutex;

sem_t		   	repartidores_libres_sem;

typedef enum { FIFO, HRRN, SJFSD } e_algoritmo;

e_algoritmo 	ALGORITMO_PLANIFICACION;

double      	ALPHA;

e_algoritmo app_obtener_algoritmo(void);

void app_iniciar_colas_planificacion(void);

void app_agregar_repartidor_esperando(t_repartidor* repartidor);
void app_liberar_repartidor(t_repartidor* repartidor);
void app_derivar_repartidor(t_repartidor* repartidor);
void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id);
t_pos app_destino_repartidor(t_repartidor* repartidor);
bool repartidor_llego_a_destino(t_repartidor* repartidor);

void app_agregar_repartidor_libre(t_repartidor* repartidor);
t_repartidor* app_obtener_repartidor_libre(t_pos destino);

bool toca_descansar(t_repartidor* repartidor);
void app_agregar_repartidor_descansando(t_repartidor* repartidor);
void descansa(t_repartidor repartidor);
void app_reviso_repartidores_descansados();

double proxima_rafaga(t_pcb* pcb);
double response_ratio(t_pcb* pcb);
void app_ordenar_ready(void);

t_repartidor* app_ready_pop(void);

#endif /* APPCORE_H_ */
