#include "appconectados.h"
#include "appenvio.h"
#include "applanificador.h"
#include "apprecepcion.h"

#define MODULE_NAME		 "APP"
#define CONFIG_FILE_PATH "app.config"
#define LOG_FILE_KEY	 "ARCHIVO_LOG"

void app_routine(void) //TODO: Cambiar!!
{
	sem_t aux;
	sem_init(&aux, 0, 0);
	sem_wait(&aux);
}

int main(int argc, char* argv[])
{
	for(int i = 1; i < argc; i++) if(string_starts_with(argv[i],"--")) cs_parse_argument(argv[i]);

	cs_module_init(CONFIG_FILE_PATH, LOG_FILE_KEY, MODULE_NAME);
	CS_LOG_TRACE("Iniciado correctamente");

	app_conectados_init();
	app_recepcion_init();
	app_inicializar_repartidores();
	app_iniciar_planificador();
	app_routine();

	cs_module_close();

	return EXIT_SUCCESS;
}
