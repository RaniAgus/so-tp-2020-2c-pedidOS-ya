#include "apprepartidores.h"

static int GRADO_DE_MULTIPROCESAMIENTO;

t_pos app_destino_repartidor(t_repartidor* repartidor);
bool repartidor_llego_a_destino(t_repartidor* repartidor);
bool app_mover_repartidor(t_repartidor* repartidor, bool alternador);

void app_iniciar_repartidores(void)
{
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

	GRADO_DE_MULTIPROCESAMIENTO = cs_config_get_int("GRADO_DE_MULTIPROCESAMIENTO");

}

/*********************************** RUTINA DEL REPARTIDOR ***********************************/

//TODO: [APP] Ver si crear un hilo por procesador o por repartidor

/********************************** POSICION DEL REPARTIDOR **********************************/

static void mover_x_repartidor(t_repartidor* repartidor, t_pos destino);
static void mover_y_repartidor(t_repartidor* repartidor, t_pos destino);
static void loggear_movimiento(t_repartidor* repartidor, t_pos anterior);

t_pos app_destino_repartidor(t_repartidor* repartidor)
{
	return repartidor->destino == DESTINO_RESTAURANTE ?
		repartidor->pcb->posicionRestaurante : repartidor->pcb->posicionCliente;
}

bool repartidor_llego_a_destino(t_repartidor* repartidor) {
	t_pos destino = app_destino_repartidor(repartidor);
	return repartidor->posicion.x == destino.x && repartidor->posicion.y == destino.y;
}

bool app_mover_repartidor(t_repartidor* repartidor, bool alternador)
{
	if(alternador) {
		mover_x_repartidor(repartidor, app_destino_repartidor(repartidor));
	} else {
		mover_y_repartidor(repartidor, app_destino_repartidor(repartidor));
	}
	return !alternador;
}

static void mover_x_repartidor(t_repartidor* repartidor, t_pos destino)
{
	t_pos anterior = repartidor->posicion;

	if(repartidor->posicion.x < destino.x) {
		repartidor->posicion.x ++;
		loggear_movimiento(repartidor, anterior);
	} else if(repartidor->posicion.x > destino.x) {
		repartidor->posicion.x --;
		loggear_movimiento(repartidor, anterior);
	} else {
		//Si el repartidor ya esta en el x del destino, que mueva la y.
		mover_y_repartidor(repartidor, destino);
	}
}

static void mover_y_repartidor(t_repartidor* repartidor, t_pos destino)
{
	t_pos anterior = repartidor->posicion;
	if(repartidor->posicion.y < destino.y) {
		repartidor->posicion.y ++;
		loggear_movimiento(repartidor, anterior);
	} else if(repartidor->posicion.y > destino.y) {
		repartidor->posicion.y --;
		loggear_movimiento(repartidor, anterior);
	} else {
		//Si el repartidor ya esta en el y del destino, que mueva la x.
		mover_x_repartidor(repartidor, destino);
	}
}

static void loggear_movimiento(t_repartidor* repartidor, t_pos anterior)
{
	CS_LOG_DEBUG("Se movió el repartidor: {ID: %d} {POS_REPARTIDOR: [%d,%d] -> [%d,%d]}"
			, repartidor->id
			, anterior.x
			, anterior.y
			, repartidor->posicion.x
			, repartidor->posicion.y
	);
}
