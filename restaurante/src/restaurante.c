#include "restclientes.h"
#include "restcore.h"
#include "restdispatcher.h"
#include "restenvio.h"
#include "restplanificador.h"
#include "restrecepcion.h"

#define CONFIG_FILE_PATH "restaurante.config"
#define LOG_FILE_KEY	 "ARCHIVO_LOG"

void restaurante_init(void)
{
	char* nombre_en_mayus;

	CHECK_STATUS(cs_config_init(CONFIG_FILE_PATH));

	mi_nombre = cs_config_get_string("NOMBRE_RESTAURANTE");
	nombre_en_mayus = strdup(mi_nombre);
	string_to_upper(nombre_en_mayus);

	CHECK_STATUS(cs_logger_init(LOG_FILE_KEY, nombre_en_mayus));
	cs_error_init();

	rest_planificador_init( rest_dispatcher_init( rest_obtener_metadata() ) );
	rest_recepcion_init();
	rest_app_connect();

	CS_LOG_TRACE("Iniciado correctamente");
	free(nombre_en_mayus);
}

void restaurante_routine(void)
{
	while(1)
	{
		rest_iniciar_ciclo_cpu();
		sleep(cs_config_get_int("RETARDO_CICLO_CPU"));
		rest_esperar_fin_ciclo_cpu();
	}
}

int main(int argc, char* argv[])
{
	for(int i = 1; i < argc; i++) if(string_starts_with(argv[i],"--")) cs_parse_argument(argv[i]);

	restaurante_init();
	restaurante_routine();

	cs_module_close();

	return EXIT_SUCCESS;
}
