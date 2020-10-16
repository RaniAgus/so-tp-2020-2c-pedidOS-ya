#include "utilsComanda.h"

void server_send_rta_ok_fail(e_msgtype msg_type, t_sfd client_conn,e_opcode ok_fail)
{
	t_header header= {ok_fail,msg_type};
	char* rta_to_str = cs_msg_to_str(NULL, header.opcode, header.msgtype);
	if( cs_send_respuesta(client_conn, header, NULL) == STATUS_SUCCESS )
		{
			CS_LOG_INFO("Se envió la respuesta: %s", rta_to_str);
		} else
		{
			CS_LOG_ERROR("No se pudo enviar la respuesta: %s", rta_to_str);
		}

		free(rta_to_str);
		cs_msg_destroy(NULL, header.opcode, header.msgtype);

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

t_pagina* buscarPlato(t_segmentoPedido* unPedido,char* comida){
	int tamanioComidas = list_size(unPedido->tablaPaginas);
	char* comidaExtraida = malloc(24);
	t_pagina* plato;
	for (int i=0;i<tamanioComidas;i++){
		 plato = list_get(unPedido->tablaPaginas,i);
		 memcpy(comidaExtraida, (plato->inicioMemoria)+8,24);
		 if(!strcmp(comidaExtraida,comida)){
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

void liberarRespuestaObtener(t_rta_obt_ped* respuestaObtener){
	int tamanioLista = list_size(respuestaObtener->platos_y_estados);
	for(int i=0; i<tamanioLista;i++){
		t_plato* platoYestado = list_get(respuestaObtener->platos_y_estados,i);
		free(platoYestado->comida);
		free(platoYestado);
	}
	list_destroy(respuestaObtener->platos_y_estados);
	free(respuestaObtener);
}




t_list* acomodarFrames(int tamMemoria){
	int cantidadDeFrames = tamMemoria/32;
	t_list* frames = list_create();
	int offset = 0;
	for(int i=0; i<cantidadDeFrames; i++){
		t_frame_en_memoria* unFrame = malloc(sizeof(t_frame_en_memoria));
		unFrame->inicio = memoriaPrincipal + offset;
		offset+=32;
		unFrame->paginaALaQuePertenece = NULL;
		list_add(frames,unFrame);
	}
//	printf("LA memoria incia en %i, el ultimo frame esta en en %i", (int)memoriaPrincipal, (int) (((t_frame_en_memoria*)list_get(frames,31))->inicio));
	return frames;
}

t_frame_en_memoria* dameUnFrame(){
	int tamanioListaFrames = list_size(listaFramesMemoria);
	for (int i=0;i<tamanioListaFrames;i++){
		t_frame_en_memoria* unFrame;
		unFrame = list_get(listaFramesMemoria,i);
		if(unFrame->paginaALaQuePertenece == NULL){
			CS_LOG_TRACE("le di la posicion de memoria %i", (int) unFrame->inicio);
			return unFrame;
		}
	}
	//TODO: Swaaaaaaaaaaap
}

void liberarFrame(void* direccion){
	int tamanioFrames = list_size(listaFramesMemoria);
	for(int i=0;i<tamanioFrames;i++){
		t_frame_en_memoria* unFrame = list_get(listaFramesMemoria,i);
		if(unFrame->inicio == direccion){
			CS_LOG_TRACE("me libere un frame");
			unFrame->paginaALaQuePertenece = NULL;
			break;
		}
	}
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
		t_segmentoPedido* pedido = malloc(sizeof(t_segmentoPedido));
		pedido->idPedido = msg->pedido_id;
		pedido->tablaPaginas = list_create();
		pedido->estadoPedido = PEDIDO_PENDIENTE;
		list_add(restaurante->pedidos,pedido);
		return OPCODE_RESPUESTA_OK;
	} else{
		CS_LOG_TRACE("encontre el restaurante, voy a crear el pedido");
		t_segmentoPedido* pedido = buscarPedido(msg->pedido_id, restaurante);
		if(pedido){ //si encontro el pedido
			return OPCODE_RESPUESTA_FAIL;
		}
		pedido = malloc(sizeof(t_segmentoPedido));
		pedido->idPedido = msg->pedido_id;
		pedido->tablaPaginas = list_create();
		pedido->estadoPedido = PEDIDO_PENDIENTE;
		list_add(restaurante->pedidos,pedido);
		return OPCODE_RESPUESTA_OK;
	}
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
	if(pedido->estadoPedido != PEDIDO_PENDIENTE){
		return OPCODE_RESPUESTA_FAIL;
	}
	pthread_mutex_lock(&mutexMemoriaInterna);
	t_pagina* plato = buscarPlato(pedido,msg->comida);
	if(plato){
		uint32_t dondeDepositoLaLectura;
		memcpy(&dondeDepositoLaLectura,plato->inicioMemoria,sizeof(uint32_t));
		dondeDepositoLaLectura+= msg->cantidad;
		memcpy(plato->inicioMemoria,&dondeDepositoLaLectura,sizeof(uint32_t));
	} else{
		t_frame_en_memoria* frameAPisar = dameUnFrame();
		t_pagina* pagina = malloc(sizeof(t_pagina));
		pagina->inicioMemoria = frameAPisar->inicio;
		pagina->numeroPagina= list_size(pedido->tablaPaginas);
		pagina->presente =1;
		frameAPisar->paginaALaQuePertenece = pagina;
		int offset=0;
		memcpy(pagina->inicioMemoria + offset,&(msg->cantidad),sizeof(int));
		offset+=sizeof(uint32_t);
		int cero = 0;
		memcpy(pagina->inicioMemoria + offset,&cero,sizeof(int));
		offset+=sizeof(uint32_t);
		memcpy(pagina->inicioMemoria+offset,msg->comida,strlen(msg->comida)+1);
		list_add(pedido->tablaPaginas,pagina);

	}
	pthread_mutex_unlock(&mutexMemoriaInterna);
	//TODO: cosas de swap //4
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
	int tamanioListaPaginas = list_size(pedido->tablaPaginas);
	for(int i=0;i<tamanioListaPaginas;i++){
		pagina = list_get(pedido->tablaPaginas,i);
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
	t_rta_obt_ped* retorno;
	if(tamanioListaPaginas > 0){
		char *comidas, *listos, *totales;
		cs_platos_to_string(listaPlatos,&comidas,&listos,&totales);
		retorno =  cs_rta_obtener_ped_create(pedido->estadoPedido,comidas,listos,totales);
		free(comidas);
		free(listos);
		free(totales);
	} else{
		retorno =  cs_rta_obtener_ped_create(pedido->estadoPedido,"[]","[]","[]"); //TODO: cuando mando esto, el cliente hace seg fault
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
	if(pedido->estadoPedido != PEDIDO_PENDIENTE){
		return OPCODE_RESPUESTA_FAIL;
	}
	pedido->estadoPedido = PEDIDO_CONFIRMADO;
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
	t_pagina* plato = buscarPlato(pedido,msg->comida);
	if(!plato){
		return OPCODE_RESPUESTA_FAIL;
	}
	if(pedido->estadoPedido != PEDIDO_CONFIRMADO){
		return OPCODE_RESPUESTA_FAIL;
	}
	pthread_mutex_lock(&mutexMemoriaInterna);
	int dondeDepositoLaLectura;
	memcpy(&dondeDepositoLaLectura,(plato->inicioMemoria) +4,sizeof(uint32_t));
	dondeDepositoLaLectura++;
	memcpy((plato->inicioMemoria) + 4,&dondeDepositoLaLectura,sizeof(uint32_t));
	e_opcode retorno = buscarYTerminarPedido(pedido);
	pthread_mutex_unlock(&mutexMemoriaInterna);
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

	if(pedido->estadoPedido != PEDIDO_TERMINADO){
		return OPCODE_RESPUESTA_FAIL;
	}
	int tamanioPaginas = list_size (pedido->tablaPaginas);
	for(int i=0;i<tamanioPaginas;i++){
		t_pagina* unaPagina = list_get(pedido->tablaPaginas,i);
		if(unaPagina->presente){
			pthread_mutex_lock(&mutexMemoriaInterna);
			liberarFrame(unaPagina->inicioMemoria);
			pthread_mutex_unlock(&mutexMemoriaInterna);
		}
		free(unaPagina);
	}
	list_destroy(pedido->tablaPaginas);
	borrarPedidoDeRestaurante(msg->pedido_id,unRest);
	free(pedido);
	return OPCODE_RESPUESTA_OK;
}




