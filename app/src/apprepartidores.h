#ifndef APPCORE_H_
#define APPCORE_H_

#include "appconectados.h"
#include "appenvio.h"

#endif /* APPCORE_H_ */

typedef enum{
	NUEVO,
	LISTO,
	BLOQUEADO,
	EJECUTANDO,
	FIN
} e_estados;

typedef struct{
	uint32_t id;
	t_pos posicion;
	int frecuencia_de_descanso;
	int tiempo_de_descanso;
	e_estados estado;
} t_repartidor;

t_list* Repartidores;



