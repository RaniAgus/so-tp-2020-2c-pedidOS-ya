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
	e_destinos destino;
} t_pcb;

typedef struct {
	t_pcb* pcb;
	uint32_t id;
	t_pos posicion;
	int ciclos_sin_descansar;
	int frecuencia_de_descanso;
	int tiempo_de_descanso;
	double estimacion_anterior;
	double ultima_rafaga;
	double espera;
} t_repartidor;

void app_iniciar_colas_planificacion(void);

void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id);

void app_agregar_repartidor_libre(t_repartidor* repartidor);
t_repartidor* app_obtener_repartidor_libre(t_pos destino);

static void   app_ready_push(t_pcb* pcb);
t_repartidor* app_ready_pop(void);

void app_derivar_repartidor(t_repartidor* repartidor);

#endif /* APPCORE_H_ */
