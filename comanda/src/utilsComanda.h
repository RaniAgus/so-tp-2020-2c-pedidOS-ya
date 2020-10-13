/*
 * utilsComanda.h
 *
 *  Created on: 3 oct. 2020
 *      Author: utnso
 */

#ifndef UTILSCOMANDA_H_
#define UTILSCOMANDA_H_

#include <cshared/cshared.h>

void* memoriaPrincipal;
t_list* listaRestaurantes;
t_list* listaFramesMemoria;

typedef struct
{
	void* inicioMemoria;
	uint8_t presente;
	uint32_t numeroPagina;
}t_pagina;

typedef struct
{
	void* inicio;
	t_pagina* paginaALaQuePertenece; //estaOcupado
}t_frame_en_memoria;

typedef struct
{
	t_list* tablaPaginas;
	uint32_t idPedido;
	e_estado_ped estadoPedido;
}t_segmentoPedido;



typedef struct
{
	char* nombreRestaurante;
	t_list* pedidos;
}t_restaurante;

e_opcode guardarPedido(t_consulta* msg);
e_opcode guardarPlato(t_consulta* msg);

t_restaurante* buscarRestaurante(char* restaurante);
t_list* acomodarFrames(int tamMemoria);

#endif /* UTILSCOMANDA_H_ */
