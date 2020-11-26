#include "utilsComanda.h"

void actualizarLRU(t_frame_en_swap* frameEnSwap) {
	pthread_mutex_lock(&mutexLRU);
	frameEnSwap->LRU = ++contadorLRU;
	pthread_mutex_unlock(&mutexLRU);
}

t_restaurante* buscarRestaurante(char* restaurante){
	int tamanioSegmentos= list_size(listaRestaurantes);
	t_restaurante* unRestaurante;
	for (int i=0; i<tamanioSegmentos; i++){
		unRestaurante = list_get(listaRestaurantes,i);
		//printf("voy a comparar %s, %s",unRestaurante->nombreRestaurante,restaurante);
		if(!strcmp((unRestaurante->nombreRestaurante),restaurante)){
			return unRestaurante;
			CS_LOG_TRACE("voy a retornar un restaurante bueno");
		}
	}
	return (t_restaurante*) 0;
}

t_segmentoPedido* buscarPedido(uint32_t pedido_id, t_restaurante* unRest){
	int tamListaPedidos = list_size(unRest->pedidos);
	for (int i=0;i<tamListaPedidos;i++){
		t_segmentoPedido* unPedido = list_get(unRest->pedidos,i);
		if (unPedido->idPedido == pedido_id){
			return unPedido;
		}
	}
	return NULL;
}

t_pagina* buscarPlato(t_segmentoPedido* unPedido, char* comida) {
	int tamanioComidas = list_size(unPedido->tablaPaginas);
	char* comidaExtraida = malloc(24);
	t_pagina* plato;
	for (int i = 0; i < tamanioComidas; i++) {
		plato = list_get(unPedido->tablaPaginas, i);
		if (!(plato->frameEnSwap->presente)) {
			traemeDeSwap(plato->frameEnSwap);
		}
		plato->inicioMemoria = plato->frameEnSwap->frameAsignado->inicio;

		memcpy(comidaExtraida, (plato->inicioMemoria) + 8, 24);
		actualizarLRU(plato->frameEnSwap);
		if (!strcmp(comidaExtraida, comida)) {
			free(comidaExtraida);
			return plato;
		}
	}
	free(comidaExtraida);

	return NULL;
}

e_opcode buscarYTerminarPedido(t_segmentoPedido* pedido){
	int tamanioComidas = list_size(pedido->tablaPaginas);
	t_pagina* plato;
	uint32_t total;
	uint32_t listos;
	for (int i=0;i<tamanioComidas;i++){
		plato = list_get(pedido->tablaPaginas,i);
		memcpy(&total,plato->inicioMemoria,sizeof(uint32_t));
		memcpy(&listos,(plato->inicioMemoria) +4,sizeof(uint32_t));
		if(listos<total){
			return OPCODE_RESPUESTA_OK;
		}
		if(listos>total){
			return OPCODE_RESPUESTA_FAIL;
		}
	}
	pedido->estadoPedido = PEDIDO_TERMINADO;
	return OPCODE_RESPUESTA_OK;
}

void borrarPedidoDeRestaurante(uint32_t pedido_id, t_restaurante* unRest){
	int tamListaPedidos = list_size(unRest->pedidos);
	for (int i=0;i<tamListaPedidos;i++){
		t_segmentoPedido* unPedido = list_get(unRest->pedidos,i);
		if (unPedido->idPedido == pedido_id){
			list_remove(unRest->pedidos,i);
			break;
		}
	}
}

t_list* acomodarFrames(int tamMemoria){
	t_list* frames = list_create();
	for(int offset = 0; offset<tamMemoria; offset+=32){
		t_frame_en_memoria* unFrame = malloc(sizeof(t_frame_en_memoria));
		unFrame->inicio = memoriaPrincipal + offset;
		//unFrame->paginaALaQuePertenece = NULL;
		unFrame->estaSiendoUsado=0;
		list_add(frames,unFrame);
	}
//	printf("LA memoria incia en %i, el ultimo frame esta en en %i", (int)memoriaPrincipal, (int) (((t_frame_en_memoria*)list_get(frames,31))->inicio));
	return frames;
}

t_frame_en_memoria* dameUnFrameEnMemoria(){
	pthread_mutex_lock(&mutexListaFrames);
	int tamanioListaFrames = list_size(listaFramesMemoria);
	for (int i=0;i<tamanioListaFrames;i++){
		t_frame_en_memoria* unFrame;
		unFrame = list_get(listaFramesMemoria,i);
		if(unFrame->estaSiendoUsado == 0){
			CS_LOG_TRACE("le di la posicion de memoria %i", (int) unFrame->inicio);
			pthread_mutex_unlock(&mutexListaFrames);
			return unFrame;
		}
	}
	pthread_mutex_unlock(&mutexListaFrames);

	return NULL;
}

t_frame_en_swap* dameUnFrameEnSwap(){
	pthread_mutex_lock(&mutexAreaSwap);
	int tamanioListaFramesEnSwap = list_size(listaFramesEnSwap);
	for (int i=0;i<tamanioListaFramesEnSwap;i++){
		t_frame_en_swap* unFrame;
		unFrame = list_get(listaFramesEnSwap,i);
		if(unFrame->frameAsignado == NULL){
			CS_LOG_TRACE("le di la posicion de swap %i", (int) unFrame->inicio);
			unFrame->frameAsignado = dameUnFrameEnMemoria();
			pthread_mutex_unlock(&mutexAreaSwap);
			if(unFrame->frameAsignado == NULL){
				traemeDeSwap(unFrame);
			}
			return unFrame;
		}
	}
	pthread_mutex_unlock(&mutexAreaSwap);
	return NULL;
}

void liberarFrame(void* direccion){
	pthread_mutex_lock(&mutexListaFrames);
	int tamanioFrames = list_size(listaFramesMemoria);
	for(int i=0;i<tamanioFrames;i++){
		t_frame_en_memoria* unFrame = list_get(listaFramesMemoria,i);
		if(unFrame->inicio == direccion){
			CS_LOG_TRACE("me libere un frame");
			unFrame->estaSiendoUsado = 0;
			break;
		}
	}
	pthread_mutex_unlock(&mutexListaFrames);
}

t_list* crearAreaSwap(int tamSwap){
	int fd;
	//struct stat file_st;
	fd= open("swap.swp",O_RDWR | O_CREAT, (mode_t) 0777);
	if(fd==-1){
		CS_LOG_ERROR("error abriendo swap.bin");
		exit(-1);
	}

	fallocate(fd, 0, 0, cs_config_get_int("TAMANIO_SWAP"));
	areaSwap= mmap(NULL, cs_config_get_int("TAMANIO_SWAP"),PROT_WRITE | PROT_READ,MAP_SHARED,fd,0);
	if(areaSwap==MAP_FAILED){
		CS_LOG_ERROR("error mapeando");
		exit(-1);
	}

	t_list* framesEnSwap = list_create();
	for(int offset=0; offset < tamSwap; offset+=32){
		t_frame_en_swap* unFrame = malloc(sizeof(t_frame_en_swap));
		unFrame->inicio = areaSwap + offset;
		unFrame->frameAsignado = NULL;
		unFrame->presente =0;
		unFrame->LRU =1;
		list_add(framesEnSwap,unFrame);
	}
	return framesEnSwap;

}

t_frame_en_swap* buscarUM() {
	t_frame_en_swap* encontrado = NULL;

	int inicio = punteroClock;
	uint32_t bitModificadoBuscado = 0;

	while(encontrado == NULL) {
		do {
			t_frame_en_memoria* siguienteFrame = list_get(listaFramesMemoria, punteroClock);
			bool mismoFrame(t_frame_en_swap* potencialFrameEnSwap) {
				return potencialFrameEnSwap->presente && potencialFrameEnSwap->frameAsignado == siguienteFrame;
			}
			t_frame_en_swap* potencialFrame = list_find(listaFramesEnSwap, mismoFrame);
			CS_LOG_TRACE("y el frame a reemplazar tiene presente tiene modificado = %i, usado = %i",potencialFrame->modificado,potencialFrame->usado);


			if(bitModificadoBuscado == 0) {
				if(potencialFrame->modificado == 0 && potencialFrame->usado == 0) {
					encontrado = potencialFrame;
				}
			} else {
				if(potencialFrame->modificado == 1 && potencialFrame->usado == 0) {
					encontrado = potencialFrame;
				} else {
					potencialFrame->usado = 0;
				}
			}
			punteroClock = (punteroClock + 1) % list_size(listaFramesMemoria);
		} while(punteroClock != inicio && encontrado == NULL);
		bitModificadoBuscado = !bitModificadoBuscado;
	}

	return encontrado;
}

t_frame_en_memoria* liberameUnFrameEnMemoria() {
	t_frame_en_swap* frameAReemplazar = NULL;

	if (!strcmp(cs_config_get_string("ALGORITMO_REEMPLAZO"), "LRU")) {
		CS_LOG_INFO("Voy a reemplazar por LRU");
		int compararLRU(t_frame_en_swap* frame1, t_frame_en_swap* frame2) {
			CS_LOG_TRACE("EL LRU esta en %i, el min es %i",frame2->LRU, frame1->LRU);
			return frame1->LRU - frame2->LRU;
		}

		bool estaPresente(t_frame_en_swap* unFrameEnSwap){
			return unFrameEnSwap->presente;
		}
		t_list* framesEnSwapPresentes = list_filter(listaFramesEnSwap, (void*) estaPresente);
		frameAReemplazar = list_get_min_by(framesEnSwapPresentes, (void*) compararLRU);
		list_destroy(framesEnSwapPresentes);
	} else {
		CS_LOG_INFO("Voy a reemplazar por Clock Modificado");
		frameAReemplazar = buscarUM();
	}
	CS_LOG_TRACE("y el frame a reemplazar tiene presente tiene modificado = %i, usado = %i",frameAReemplazar->modificado,frameAReemplazar->usado);

	CS_LOG_INFO("La victima para reemplazo de pagina es el plato %s", (char*) frameAReemplazar->frameAsignado->inicio + 8);

	if(frameAReemplazar->modificado){
		memcpy(frameAReemplazar->inicio,frameAReemplazar->frameAsignado->inicio,32);
	}
	frameAReemplazar->modificado=0;
	frameAReemplazar->presente=0;
	frameAReemplazar->usado=0;

	return frameAReemplazar->frameAsignado;
}

void traemeDeSwap(t_frame_en_swap* frameEnSwap) {
	pthread_mutex_lock(&mutexAreaSwap);
	t_frame_en_memoria* frameLibre = dameUnFrameEnMemoria();

	if(frameLibre!=NULL) {
		CS_LOG_TRACE ("Encontre un frame vacio");
		frameLibre->estaSiendoUsado=1;
	} else {
		frameLibre = liberameUnFrameEnMemoria();
	}

	memcpy(frameLibre->inicio,frameEnSwap->inicio,32);
	frameEnSwap->frameAsignado = frameLibre;
	actualizarLRU(frameEnSwap);
	frameEnSwap->presente =1;
	frameEnSwap->modificado=0;
	frameEnSwap->usado= 1;
	pthread_mutex_unlock(&mutexAreaSwap);

}


e_opcode guardarPedido(t_consulta* msg){
	t_restaurante* restaurante;
	restaurante = buscarRestaurante(msg->restaurante);
	if(!restaurante){
		CS_LOG_TRACE("no encontre el restaurante");
		restaurante = malloc(sizeof(t_restaurante));
		restaurante->nombreRestaurante = string_duplicate(msg->restaurante);
		restaurante->pedidos = list_create();
		list_add(listaRestaurantes,restaurante);
	} else{
		CS_LOG_TRACE("encontre el restaurante, voy a crear el pedido");
		if(buscarPedido(msg->pedido_id, restaurante) != NULL) { //si encontro el pedido
			return OPCODE_RESPUESTA_FAIL;
		}
	}

	t_segmentoPedido* pedido = malloc(sizeof(t_segmentoPedido));
	pedido->idPedido = msg->pedido_id;
	pedido->tablaPaginas = list_create();
	pedido->estadoPedido = PEDIDO_PENDIENTE;
	pthread_mutex_init(&(pedido->mutexPedido),NULL);
	list_add(restaurante->pedidos,pedido);
	return OPCODE_RESPUESTA_OK;
}

e_opcode guardarPlato(t_consulta* msg){
	t_restaurante* unRest = buscarRestaurante(msg->restaurante); //1
	if(!unRest){
		return OPCODE_RESPUESTA_FAIL;
	}
	t_segmentoPedido* pedido = buscarPedido(msg->pedido_id,unRest); //2
	if(!pedido){
		return OPCODE_RESPUESTA_FAIL;
	}
	pthread_mutex_lock(&(pedido->mutexPedido));
	if(pedido->estadoPedido != PEDIDO_PENDIENTE){
		pthread_mutex_unlock(&(pedido->mutexPedido));
		return OPCODE_RESPUESTA_FAIL;
	}
	pthread_mutex_lock(&mutexMemoriaInterna);
	t_pagina* plato = buscarPlato(pedido,msg->comida);
	if(plato){
		if(!(plato->frameEnSwap->presente)){
			traemeDeSwap(plato->frameEnSwap);
		}
		plato->inicioMemoria = plato->frameEnSwap->frameAsignado->inicio;

		uint32_t dondeDepositoLaLectura;
		memcpy(&dondeDepositoLaLectura,plato->inicioMemoria,sizeof(uint32_t));
		dondeDepositoLaLectura+= msg->cantidad;
		memcpy(plato->inicioMemoria,&dondeDepositoLaLectura,sizeof(uint32_t));

		actualizarLRU(plato->frameEnSwap);
		plato->frameEnSwap->usado=1;
		plato->frameEnSwap->modificado =1;
	} else{
		t_frame_en_swap* enSwap=dameUnFrameEnSwap();
		if(enSwap == NULL){
			CS_LOG_INFO("Tengo swap lleno");
			pthread_mutex_unlock(&mutexMemoriaInterna);
			pthread_mutex_unlock(&(pedido->mutexPedido));
			return OPCODE_RESPUESTA_FAIL;
		}
		t_pagina* pagina = malloc(sizeof(t_pagina));
		pthread_mutex_lock(&mutexAreaSwap);
		pagina->frameEnSwap = enSwap;
		pagina->inicioMemoria = enSwap->frameAsignado->inicio;
		pagina->numeroPagina= list_size(pedido->tablaPaginas);
		pagina->frameEnSwap->presente=1;
		enSwap->frameAsignado->estaSiendoUsado = 1;
		enSwap->modificado =0;
		enSwap->usado=1;
		pthread_mutex_unlock(&mutexAreaSwap);

		actualizarLRU(pagina->frameEnSwap);

		int offset=0;
		memcpy(pagina->inicioMemoria + offset,&(msg->cantidad),sizeof(uint32_t));
		offset+=sizeof(uint32_t);
		uint32_t cero = 0;
		memcpy(pagina->inicioMemoria + offset,&cero,sizeof(uint32_t));
		offset+=sizeof(uint32_t);
		memcpy(pagina->inicioMemoria+offset,msg->comida,strlen(msg->comida)+1);
		memcpy(enSwap->inicio,pagina->inicioMemoria,32);
		list_add(pedido->tablaPaginas,pagina);
		CS_LOG_INFO("Guarde el plato %s en la direccion de memoria %i",msg->comida, pagina->inicioMemoria);


	}
	pthread_mutex_unlock(&mutexMemoriaInterna);
	pthread_mutex_unlock(&(pedido->mutexPedido));
	return OPCODE_RESPUESTA_OK;


}


t_rta_obt_ped* obtenerPedido(t_consulta* msg){
	t_restaurante* restaurante = buscarRestaurante(msg->restaurante);
	if(!restaurante){
		return NULL;
	}
	t_segmentoPedido* pedido = buscarPedido(msg->pedido_id,restaurante);
	if(!pedido){
	//	return cs_rta_obtener_ped_create(PEDIDO_INVALIDO, "[]","[]","[]");
		return NULL;
	}
	t_list* listaPlatos = list_create();
	t_pagina* pagina;
	pthread_mutex_lock(&(pedido->mutexPedido));
	int tamanioListaPaginas = list_size(pedido->tablaPaginas);
	for(int i=0;i<tamanioListaPaginas;i++){
		pagina = list_get(pedido->tablaPaginas,i);
		CS_LOG_TRACE("EStoy presente %i", pagina->frameEnSwap->presente);

		if(!(pagina->frameEnSwap->presente)){
			traemeDeSwap(pagina->frameEnSwap);
		}
		pagina->inicioMemoria = pagina->frameEnSwap->frameAsignado->inicio;

		actualizarLRU(pagina->frameEnSwap);
		pagina->frameEnSwap->usado=1;
		t_plato* unPlato = malloc(sizeof(t_plato));
		pthread_mutex_lock(&mutexMemoriaInterna);
		memcpy(&unPlato->cant_total,(pagina->inicioMemoria),sizeof(uint32_t));
		memcpy(&unPlato->cant_lista,(pagina->inicioMemoria)+4,sizeof(uint32_t));
		char* temp = malloc(24);
		memcpy(temp,(pagina->inicioMemoria) + 8,24);
		pthread_mutex_unlock(&mutexMemoriaInterna);
		unPlato->comida = string_duplicate(temp);
		free(temp);
		list_add(listaPlatos,unPlato);
		//puts(unPlato->comida);
	}
	pthread_mutex_unlock(&(pedido->mutexPedido));
	t_rta_obt_ped* retorno;
	if(tamanioListaPaginas > 0){
		char *comidas, *listos, *totales;
		cs_platos_to_string(listaPlatos,&comidas,&listos,&totales);
		retorno =  cs_rta_obtener_ped_create(pedido->estadoPedido,comidas,listos,totales);
		free(comidas);
		free(listos);
		free(totales);
	} else{
		retorno =  cs_rta_obtener_ped_create(pedido->estadoPedido,"[]","[]","[]");
	}


	for(int i=0;i<tamanioListaPaginas;i++){
		t_plato* unPlato = list_get(listaPlatos,i);
		free(unPlato->comida);
		free(unPlato);
	}
	list_destroy(listaPlatos);
	return retorno;
}


e_opcode confirmarPedido(t_consulta* msg){
	t_restaurante* unRest = buscarRestaurante(msg->restaurante); //1
	if(!unRest){
		return OPCODE_RESPUESTA_FAIL;
	}
	t_segmentoPedido* pedido = buscarPedido(msg->pedido_id,unRest); //2
	if(!pedido){
		return OPCODE_RESPUESTA_FAIL;
	}
	pthread_mutex_lock(&(pedido->mutexPedido));
	if(pedido->estadoPedido != PEDIDO_PENDIENTE){
		pthread_mutex_unlock(&(pedido->mutexPedido));
		return OPCODE_RESPUESTA_FAIL;
	}
	pedido->estadoPedido = PEDIDO_CONFIRMADO;
	pthread_mutex_unlock(&(pedido->mutexPedido));

	return OPCODE_RESPUESTA_OK;
}

e_opcode platoListo(t_consulta* msg){
	t_restaurante* unRest = buscarRestaurante(msg->restaurante); //1
	if(!unRest){
		return OPCODE_RESPUESTA_FAIL;
	}
	t_segmentoPedido* pedido = buscarPedido(msg->pedido_id,unRest); //2
	if(!pedido){
		return OPCODE_RESPUESTA_FAIL;
	}
	pthread_mutex_lock(&(pedido->mutexPedido));
	t_pagina* plato = buscarPlato(pedido,msg->comida);
	if(!plato){
		pthread_mutex_unlock(&(pedido->mutexPedido));
		return OPCODE_RESPUESTA_FAIL;
	}
	if(pedido->estadoPedido != PEDIDO_CONFIRMADO){
		pthread_mutex_unlock(&(pedido->mutexPedido));
		return OPCODE_RESPUESTA_FAIL;
	}
	if(!(plato->frameEnSwap->presente)){
		traemeDeSwap(plato->frameEnSwap);
	}
	plato->inicioMemoria = plato->frameEnSwap->frameAsignado->inicio;

	pthread_mutex_lock(&mutexMemoriaInterna);
	int dondeDepositoLaLectura;
	memcpy(&dondeDepositoLaLectura,(plato->inicioMemoria) +4,sizeof(uint32_t));
	dondeDepositoLaLectura++;
	memcpy((plato->inicioMemoria) + 4,&dondeDepositoLaLectura,sizeof(uint32_t));
	e_opcode retorno = buscarYTerminarPedido(pedido);
	actualizarLRU(plato->frameEnSwap);
	plato->frameEnSwap->modificado=1;
	plato->frameEnSwap->usado=1;
	pthread_mutex_unlock(&mutexMemoriaInterna);
	pthread_mutex_unlock(&(pedido->mutexPedido));
	return retorno;
}


e_opcode finalizarPedido(t_consulta* msg){
	t_restaurante* unRest = buscarRestaurante(msg->restaurante); //1
	if(!unRest){
		return OPCODE_RESPUESTA_FAIL;
	}
	t_segmentoPedido* pedido = buscarPedido(msg->pedido_id,unRest); //2
	if(!pedido){
		return OPCODE_RESPUESTA_FAIL;
	}
	pthread_mutex_lock(&(pedido->mutexPedido));
/*	if(pedido->estadoPedido != PEDIDO_TERMINADO){
		pthread_mutex_unlock(&(pedido->mutexPedido));
		return OPCODE_RESPUESTA_FAIL;
	}*/
	int tamanioPaginas = list_size (pedido->tablaPaginas);
	for(int i=0;i<tamanioPaginas;i++){
		t_pagina* unaPagina = list_get(pedido->tablaPaginas,i);
		if(unaPagina->frameEnSwap->presente){
			pthread_mutex_lock(&mutexMemoriaInterna);
			liberarFrame(unaPagina->inicioMemoria);
			pthread_mutex_unlock(&mutexMemoriaInterna);
		}
		unaPagina->frameEnSwap->modificado=0;
		unaPagina->frameEnSwap->usado=0;
		unaPagina->frameEnSwap->frameAsignado = NULL;
		CS_LOG_INFO("Liberé la direccion de memoria %i", unaPagina->inicioMemoria);
		free(unaPagina);
	}
	list_destroy(pedido->tablaPaginas);
	borrarPedidoDeRestaurante(msg->pedido_id,unRest);
	pthread_mutex_unlock(&(pedido->mutexPedido));
	pthread_mutex_destroy(&(pedido->mutexPedido));
	free(pedido);
	return OPCODE_RESPUESTA_OK;
}




