#include "apprepartidores.h"

void app_inicializar_repartidores(){

	Repartidores = list_create();
	char** paresDeCoordenadas = cs_config_get_array_value("REPARTIDORES");
	char** frecuenciasDeDescanso = cs_config_get_array_value("FRECUENCIA_DE_DESCANSO");
	char** tiemposDeDescanso = cs_config_get_array_value("TIEMPO_DE_DESCANSO");

	int cantidadDeRepartidores = 0;
	int contador = 0;
	while(tiemposDeDescanso[contador] != NULL){
		cantidadDeRepartidores++;
	}

	for(int i = 0; i<cantidadDeRepartidores; i++){
		t_repartidor* repartidor = malloc(sizeof(t_repartidor));
		repartidor->id = i+1;
		repartidor->posicion.x = atoi(string_split(paresDeCoordenadas[i], "|")[0]);
		repartidor->posicion.y = atoi(string_split(paresDeCoordenadas[i], "|")[1]);
		repartidor->frecuencia_de_descanso = atoi(frecuenciasDeDescanso[i]);
		repartidor->tiempo_de_descanso = atoi(tiemposDeDescanso[i]);
		repartidor->estado = NUEVO;
		list_add(Repartidores, repartidor);
	}

}
