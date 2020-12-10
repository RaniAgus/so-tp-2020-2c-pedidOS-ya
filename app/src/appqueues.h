#ifndef APPCORE_H_
#define APPCORE_H_

#include "appconectados.h"
#include "appenvio.h"

typedef enum {
	DESTINO_CLIENTE,
	DESTINO_RESTAURANTE
} e_destinos;

typedef struct {
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
	int ciclos_descansados;
	int tiempo_de_descanso;
} t_repartidor;

typedef enum { FIFO = 0, HRRN, SJFSD } e_algoritmo;

void app_iniciar_colas_planificacion(void);

void app_derivar_repartidor(t_repartidor* repartidor);
bool repartidor_llego_a_destino(t_repartidor* repartidor);
t_pos app_destino_repartidor(t_repartidor* repartidor);

void app_agregar_repartidor_libre(t_repartidor* repartidor);
t_repartidor* app_obtener_repartidor_libre(t_pos destino);

void app_ready_actualizar_espera(void);
t_repartidor* app_ready_pop(void);

void app_agregar_repartidor_esperando(t_repartidor* repartidor);
void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id);

#endif /* APPCORE_H_ */
