/*
 * utilsComanda.h
 *
 *  Created on: 3 oct. 2020
 *      Author: utnso
 */

#ifndef UTILSCOMANDA_H_
#define UTILSCOMANDA_H_

#define _GNU_SOURCE

#include <cshared/cshared.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void* memoriaPrincipal;
void* areaSwap;
t_list* listaRestaurantes;
t_list* listaFramesMemoria;
t_list* listaFramesEnSwap;
uint32_t contadorLRU;

pthread_mutex_t mutexMemoriaInterna;
pthread_mutex_t mutexListaFrames;
pthread_mutex_t mutexLRU;
pthread_mutex_t mutexAreaSwap;

typedef struct
{
	void* inicio;
	//t_pagina* paginaALaQuePertenece; //estaOcupado
	uint32_t estaSiendoUsado;
}t_frame_en_memoria;

typedef struct
{
	void* inicio;
	t_frame_en_memoria* frameAsignado;
	uint8_t presente;
	uint32_t LRU;
	uint8_t modificado;
	uint8_t usado;
}t_frame_en_swap;

typedef struct
{
	void* inicioMemoria;
	uint32_t numeroPagina;
	t_frame_en_swap* frameEnSwap;
}t_pagina;





typedef struct
{
	t_list* tablaPaginas;
	uint32_t idPedido;
	e_estado_ped estadoPedido;
	pthread_mutex_t mutexPedido;
}t_segmentoPedido;



typedef struct
{
	char* nombreRestaurante;
	t_list* pedidos;
}t_restaurante;

t_list* crearAreaSwap(int tamSwap);

e_opcode guardarPedido(t_consulta* msg);
e_opcode guardarPlato(t_consulta* msg);
t_rta_obt_ped* obtenerPedido(t_consulta* msg);
e_opcode confirmarPedido(t_consulta* msg);
e_opcode platoListo(t_consulta* msg);
e_opcode finalizarPedido(t_consulta* msg);

t_restaurante* buscarRestaurante(char* restaurante);
t_list* acomodarFrames(int tamMemoria);

void traemeDeSwap(t_frame_en_swap* frameEnSwap);

#endif /* UTILSCOMANDA_H_ */
