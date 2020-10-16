#include "apconn.h"
#include "apsend.h"
#include "applanificador.h"
#include "aplisten.h"

#define MODULE_NAME		 "APP"
#define CONFIG_FILE_PATH "app.config"
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

	cs_module_init(CONFIG_FILE_PATH, LOG_FILE_KEY, MODULE_NAME);
	CS_LOG_TRACE("Iniciado correctamente");

	ap_conn_init();
	ap_listen_init();
	while(1) {
		sleep(30);
		ap_finalizar_pedido("Default", 1, "Cliente1");
	}

	wait();

	cs_module_close();

	return EXIT_SUCCESS;
}
