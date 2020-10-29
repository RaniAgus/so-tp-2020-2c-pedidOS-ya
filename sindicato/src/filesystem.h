#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "sindicato.h"

// -------- STRUCT -------- //


// -------- LEVANTAR FILESYSTEM -------- //

void crearDirectorioAFIP();
void crearMetadata(char*);
void crearFiles(char*);
void crearBlocks(char*);
void generarBitmap(char*, t_config*);

// -------- MENSAJES -------- //

t_rta_cons_pl* consultarPlatos(t_consulta*);
e_opcode guardarPedido(t_consulta*);
e_opcode guardarPlato(t_consulta*);
e_opcode confirmarPedido(t_consulta*);
t_rta_obt_ped* obtenerPedido(t_consulta*); // Esta bien esto?
//e_opcode+t_rta_cons_ped obtener_pedido(t_consulta*);
t_rta_obt_rest* obtenerRestaurante(t_consulta*);
e_opcode platoListo(t_consulta*);
t_rta_obt_rec* obtenerReceta(t_consulta*);

//TERMINAR PEDIDO?

// -------- CONSOLA -------- //

char* crearRestaurante(char**);
char* crearReceta(char**);


// -------- MANEJO BITMAP -------- //

int obtenerYEscribirProximoDisponible();
void eliminarBit(int);

// -------- RECETA -------- //

t_rta_obt_rec* leerReceta(char*);

// -------- RESTAURANTE -------- //

char* obtenerPathRestaurante(char*);

// -------- AUX -------- //

int existeDirectorio(char*, int);
int cantidadDeBloques(char**);
int tamanioDeBloque(char*);
void liberar_lista(char**);

#endif /* FILESYSTEM_H_ */
