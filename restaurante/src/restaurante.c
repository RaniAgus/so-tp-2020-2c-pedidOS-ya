#include "restcore.h"
#include "restlisten.h"

#define CONFIG_FILE_PATH "restaurante.config"
#define LOG_FILE_KEY	 "ARCHIVO_LOG"

void wait(void)
{
	sem_t aux;
	sem_init(&aux, 0, 0);
	sem_wait(&aux);
}

int main(int argc, char* argv[])
{
	for(int i = 1; i < argc; i++) if(string_starts_with(argv[i],"--")) cs_parse_argument(argv[i]);

	CHECK_STATUS(cs_config_init(CONFIG_FILE_PATH));
	mi_nombre = cs_config_get_string("NOMBRE_RESTAURANTE");
	CHECK_STATUS(cs_logger_init(LOG_FILE_KEY, mi_nombre));
	cs_error_init();

	rest_core_init(rest_obtener_metadata());
	rest_listen_init();
	rest_enviar_handshake();

	CS_LOG_TRACE("Iniciado correctamente");

	wait();

	cs_module_close();

	return EXIT_SUCCESS;
}
