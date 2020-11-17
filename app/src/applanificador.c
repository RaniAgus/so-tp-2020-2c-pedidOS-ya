#include "applanificador.h"

static pthread_t thread_planificador;

static t_queue*			pcbs_nuevos;
static pthread_mutex_t 	pcbs_nuevos_mutex;
static sem_t 			pcbs_nuevos_sem;

static void app_asignar_repartidor(t_pcb* pcb);
e_algoritmo app_obtener_algoritmo();
void inicializar_sem_pcb();

void app_iniciar_planificador(void)
{
	pcbs_nuevos = queue_create();
	pthread_mutex_init(&pcbs_nuevos_mutex, NULL);
	sem_init(&pcbs_nuevos_sem, 0, 0);

	algoritmo_planificacion = app_obtener_algoritmo();
	inicializar_sem_pcb();

	repartidores_libres = list_create();
	pthread_mutex_init(&repartidores_libres_mutex, NULL);
	sem_init(&repartidores_libres_sem, 0, 0);

	//Creo el hilo que se encarga de asignar los repartidores a cada pcb nuevo
	pthread_create(&thread_planificador, NULL, (void*) app_asignar_repartidor, NULL);
	pthread_detach(thread_planificador);
}

//Al confirmar pedido, se crea el PCB correspondiente y se agrega a la lista de pcbs nuevos
void app_crear_pcb(char* cliente, char* restaurante, uint32_t pedido_id)
{
    CS_LOG_TRACE(
        "Se va a crear el PCB: {CLIENTE: %s} {RESTAURANTE: %s} {ID_PEDIDO: %d}",
        cliente, restaurante, pedido_id
    );

    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->id_pedido = pedido_id;
    pcb->cliente = cliente;
    pcb->posicionCliente = app_posicion_cliente(cliente);
    pcb->restaurante = restaurante;
    pcb->posicionRestaurante = app_posicion_restaurante(restaurante);
    pcb->destino = DESTINO_RESTAURANTE;

    //Arranca sin repartidor asignado, otro hilo lo extrae de la cola y le asigna uno
    pcb->repartidor = NULL;
    queue_sync_push(pcbs_nuevos, &pcbs_nuevos_mutex, &pcbs_nuevos_sem, pcb);
}

//********* FUNCIONES PRIVADAS

e_algoritmo app_obtener_algoritmo()
{
	char* algoritmo = cs_config_get_string("ALGORITMO_PLANIFICACION");

	if (strcmp(algoritmo, "FIFO") == 0) {return FIFO;}
	else if (strcmp(algoritmo, "HRRN") == 0) {return HRRN;}
	else if (strcmp(algoritmo, "SJFSD") == 0) {return SJFSD;}
	else {return FIFO;} //En caso que el algoritmo no sea valido, se toma FIFO por defecto.
}

t_list* pcbs_asignados()
{
	bool esta_asignado(t_pcb* pcb){
		return !pcb->repartidor == NULL;
	}
	return list_filter(pcbs_nuevos, (void*) esta_asignado);
}

bool hay_pedidos_pendientes()
{
	bool verifica = false;
	t_pcb* pcb;
	int i=0;
	sem_wait(&pcbs_nuevos_mutex);
	while(i < list_size(pcbs_nuevos)){
		pcb = list_get(pcbs_nuevos, i);
		if (pcb->estado != FIN){
			verifica = true;
			break;
		}
		i++;
	}
	sem_post(&pcbs_nuevos_mutex);
	return verifica;
}

void inicializar_sem_pcb()
{
	pcb_sem = malloc(list_size(pcbs_nuevos) * sizeof(sem_t));

	for (int i = 0; i < list_size(pcbs_nuevos); i++) {
		sem_init(&(pcb_sem[i]), 0, 0);
	}
}

void activar_hilo_de(int id)
{
	sem_post(&pcb_sem[id]);
}

void app_planificar_FIFO()
{
	while(1) {
		t_pcb* pcb;

		sem_wait(&pcbs_nuevos_sem);

		if(hay_pedidos_pendientes()){
			sem_wait(&pcbs_nuevos_mutex);

			if(!list_is_empty(pcbs_asignados())){
				pcb = list_get(pcbs_nuevos, 0);
				pcb->estado = EJECUTANDO;
				app_derivar_pcb(pcb);
				activar_hilo_de(pcb->id_pedido);
			}
			else{
				sem_post(&pcbs_nuevos_mutex);
			}
		}
		else break;
	}
}

void app_gestionar_pcb_FIFO(t_pcb* pcb)
{
	while(1){
		sem_wait(&pcb_sem[pcb->id_pedido]);

		if(pcb->estado != FIN){
			if(pcb->estado == EJECUTANDO){
				bool alternador = true;

				while (pcb->repartidor->posicion.x != pcb->posicionCliente.x ||
					   pcb->repartidor->posicion.y != pcb->posicionCliente.y) {
					if (alternador) app_mover_x_repartidor(pcb->repartidor, pcb->posicionCliente);
					else app_mover_y_repartidor(pcb->repartidor, pcb->posicionCliente);

					alternador = !alternador;

					usleep(atoi(cs_config_get_string("RETARDO_CICLO_CPU")) * 1000000);
				}
			}
			//ver bloqueo
		}
		else break;
	}
}

void app_gestionar_pcb_HRRN(t_pcb* pcb){} //TODO

void app_gestionar_pcb_SJFSD(t_pcb* pcb){} //TODO

void app_gestionar_pcb(t_pcb* pcb)
{
	switch(algoritmo_planificacion){
		case FIFO:{
			app_gestionar_pcb_FIFO(pcb);
			break;
		}
		case HRRN:{
			app_gestionar_pcb_HRRN(pcb);
			break;
		}
		case SJFSD:{
			app_gestionar_pcb_SJFSD(pcb);
			break;
		}
		default:{
			app_gestionar_pcb_FIFO(pcb);
			break;
		}
	}
}

static void app_asignar_repartidor(t_pcb* pcb)
{
	while(true)
	{
		//Extrae pcb sin asignar
		t_pcb* pcb = queue_sync_pop(pcbs_nuevos, &pcbs_nuevos_mutex, &pcbs_nuevos_sem);

		CS_LOG_DEBUG("Se extrajo el PCB: {CLIENTE: %s} {RESTAURANTE: %s} {POS_DESTINO: [%d,%d]} {ID_PEDIDO: %d}"
				, pcb->cliente
				, pcb->restaurante
				, pcb->posicionRestaurante.x
				, pcb->posicionRestaurante.y
				, pcb->id_pedido
		);

		//Espera a que haya repartidores libres
		sem_wait(&repartidores_libres_sem);

		//Asigna un repartidor más cercano al restaurante (de haber varios, se asigna el más cercano)
		pthread_mutex_lock(&repartidores_libres_mutex);

		bool menor_distancia (t_repartidor* repartidor1, t_repartidor* repartidor2) {
			t_pos vector_distancia1 = calcular_vector_distancia(repartidor1->posicion, pcb->posicionRestaurante);
			t_pos vector_distancia2 = calcular_vector_distancia(repartidor2->posicion, pcb->posicionRestaurante);

			return calcular_norma(vector_distancia1) < calcular_norma(vector_distancia2);
		}
		list_sort(repartidores_libres, (void*) menor_distancia);  //Ordeno la lista por menor distancia
		pcb->repartidor = list_remove(repartidores_libres, 0); //Me quedo con el head de la lista y asi obtengo al repartidor mas cercano disponible

		pthread_mutex_unlock(&repartidores_libres_mutex);

		CS_LOG_DEBUG("Se asignó el repartidor: {ID: %d} {POS_REPARTIDOR: [%d,%d]} {POS_DESTINO: [%d,%d]}"
				, pcb->repartidor->id
				, pcb->repartidor->posicion.x
				, pcb->repartidor->posicion.y
				, pcb->posicionRestaurante.x
				, pcb->posicionRestaurante.y
		);

		//Deriva el pcb a la queue correspondiente (ready/bloqueado/etc)
		app_derivar_pcb(pcb);

		//Empieza a planificar el pcb
		sem_post(&pcbs_nuevos_sem);
	}
}

