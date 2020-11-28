#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "sindicato.h"

// -------- LEVANTAR FILESYSTEM -------- //

void crearDirectorioAFIP();
void crearMetadata(char*);
void crearFiles(char*);
void crearBlocks(char*);
void generarBitmap(char*);

// -------- MENSAJES -------- //

t_rta_cons_pl* consultarPlatos(t_consulta*);
e_opcode guardarPedido(t_consulta*);
e_opcode guardarPlato(t_consulta*);
e_opcode confirmarPedido(t_consulta*);
t_rta_obt_ped* obtenerPedido(t_consulta*);
t_rta_obt_rest* obtenerRestaurante(t_consulta*);
e_opcode platoListo(t_consulta*);
t_rta_obt_rec* obtenerReceta(t_consulta*);
e_opcode terminarPedido(t_consulta*);

//TERMINAR PEDIDO?

// -------- CONSOLA -------- //

void crearRestaurante(char**);
void crearReceta(char**);


// -------- MANEJO BITMAP -------- //

int obtenerYEscribirProximoDisponible(char*);
void eliminarBit(int);

// -------- RECETA -------- //

t_rta_obt_rec* cs_string_to_receta(char*);

// -------- PEDIDO -------- //

t_rta_obt_ped* leerPedido(char*);
int existePedido(int, char*);
void escribirInfoPedido(char*, int, char*);
int obtenerBlockInicialPedido(int, char*);
int obtenerTamanioPedido(int, char*);
e_estado_ped cs_string_to_est_ped(char*);
int estaEnEstado(char*, e_estado_ped);
char* cs_est_ped_to_string(e_estado_ped);

// -------- PISAR BLOQUES -------- //

void pisar(t_list*, char*, char*);

// -------- MODIFICAR STRINGS -------- //

char* agregarCantPlatos(char*, t_consulta*);
char* agregarPlato(char*, t_consulta*);
char* agregarPlatoListo(char*, t_consulta*);
char* cambiarEstadoPedidoA(char*, t_consulta*, e_estado_ped);

// -------- BLOQUES -------- //

int escribirBloques(char*, char*);
void escribirBloque(char*, int);
char* leerBloques(int, int);
t_list* leerNumerosBloques(int, int);
void limpiarBloque(int);

// -------- RESTAURANTE -------- //

char* obtenerPlatos(char*);
char* obtenerPathRestaurante(char*);
uint32_t obtenerCantidadPedidos(char*);
t_rta_obt_rest* cs_string_to_restaurante(char*, uint32_t);


// -------- AUX -------- //

int existeDirectorio(char*, int);
int cantidadDeBloques(char**);
int tamanioDeBloque(char*);
void liberar_lista(char**);
int tamanioArchivo(char*);

#endif /* FILESYSTEM_H_ */
