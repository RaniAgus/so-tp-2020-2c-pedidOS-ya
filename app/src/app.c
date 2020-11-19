#include "appconectados.h"
#include "appenvio.h"
#include "applanificador.h"
#include "apprecepcion.h"
#include "appqueues.h"
#include "apprepartidores.h"

#define MODULE_NAME		 "APP"
#define CONFIG_FILE_PATH "app.config"
#define LOG_FILE_KEY	 "ARCHIVO_LOG"

void app_routine(void)
{
	while(true)
	{
		app_iniciar_ciclo_cpu();
		sleep(cs_config_get_int("RETARDO_CICLO_CPU"));
		app_esperar_fin_ciclo_cpu();
	}
}

int main(int argc, char* argv[])
{
	for(int i = 1; i < argc; i++) if(string_starts_with(argv[i],"--")) cs_parse_argument(argv[i]);

	cs_module_init(CONFIG_FILE_PATH, LOG_FILE_KEY, MODULE_NAME);
	CS_LOG_TRACE("Iniciado correctamente");

	app_conectados_init();
	app_recepcion_init();
	app_iniciar_planificador_largo_plazo();
	app_iniciar_colas_planificacion();
	app_iniciar_repartidores();
	app_routine();

	cs_module_close();

	return EXIT_SUCCESS;
}
