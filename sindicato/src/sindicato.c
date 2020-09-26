#include "sindicato.h"
#include "funcionesAuxiliares.h"

int main(void) {
	leerConfig();

}

t_log* iniciar_logger(void){
	return log_create(rutaLog, " [SINDICATO] " , 1, LOG_LEVEL_INFO);
}

void leerConfig(){
	config = config_create("gamecard.config");
	rutaLog = config_get_string_value(config, "RUTA_LOG");
	magicNumber = config_get_string_value(config, "MAGIC_NUMBER");
	puntoMontaje = config_get_string_value(config,"PUNTO_MONTAJE_TALLGRASS");
}


