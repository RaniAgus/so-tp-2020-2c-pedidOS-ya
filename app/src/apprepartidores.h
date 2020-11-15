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

t_list* 		repartidores_libres;
pthread_mutex_t repartidores_libres_mutex;
sem_t			repartidores_libres_sem;

void app_inicializar_repartidores(void);

void app_avisar_pedido_terminado(char* restaurante, uint32_t pedido_id);
void app_derivar_pcb(t_pcb* pcb);

#endif /* APPCORE_H_ */
