#include "sindicato.h"
#include "filesystem.h"

int main(void) {
	leerConfig();
	logger = iniciar_logger();
	if(!strcmp(magicNumber, "AFIP")){
		crearDirectorioAFIP();
	}
}

t_log* iniciar_logger(void){
	return log_create(rutaLog, " [SINDICATO] " , 1, LOG_LEVEL_INFO);
}

void leerConfig(){
	config = config_create("sindicato.config");
	rutaLog = config_get_string_value(config, "RUTA_LOG");
	magicNumber = config_get_string_value(config, "MAGIC_NUMBER");
	puntoMontaje = config_get_string_value(config,"PUNTO_MONTAJE_TALLGRASS");
	tamanioBloque = config_get_int_value(config, "BLOCKS_SIZE");
	cantidadBloques = config_get_int_value(config, "BLOCKS");
}


