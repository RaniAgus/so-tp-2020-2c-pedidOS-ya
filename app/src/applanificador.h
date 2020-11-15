#ifndef APPLANIFICADOR_H_
#define APPLANIFICADOR_H_

#include <math.h>
#include "appconectados.h"

typedef enum {
	NUEVO,
	LISTO,
	BLOQUEADO,
	EJECUTANDO,
	FIN
} e_estados;

typedef struct {
	uint32_t id;
	t_pos posicion;
	int frecuencia_de_descanso;
	int tiempo_de_descanso;
} t_repartidor;

typedef enum {
	DESTINO_CLIENTE,
	DESTINO_RESTAURANTE
} e_destinos;

typedef struct {
	e_estados estado;
	uint32_t id_pedido;
	t_repartidor* repartidor;
	char* cliente;
	t_pos posicionCliente;
	char* restaurante;
	t_pos posicionRestaurante;
	e_destinos destino;
} t_pcb;

void app_iniciar_planificador(void);
void app_agregar_repartidor_disponible(t_repartidor* repartidor);

void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id);
void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id);

void app_derivar_pcb(t_pcb* pcb);

#endif /* APPLANIFICADOR_H_ */
