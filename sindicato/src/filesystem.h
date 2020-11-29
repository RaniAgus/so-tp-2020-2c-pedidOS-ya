#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "sindicato.h"

typedef enum {
	RES_NOMBRE = 1,
	RES_CANT_COCINEROS = 2,
	RES_POSICION = 3,
	RES_AFINIDADES = 4,
	RES_PLATOS = 5,
	RES_PRECIOS = 6,
	RES_CANT_HORNOS = 7
}e_crearRestaurante;

typedef enum {
	REC_NOMBRE = 1,
	REC_PASOS = 2,
	REC_TIEMPOS = 3
}e_crearReceta;

// ---------- FUNCIONES GENERICAS ---------- //

// -------- LEVANTAR FILESYSTEM -------- //

void crearDirectorioAFIP();
void obtenerMetadata();
void crearFiles();
void crearBlocks();
void generarBitmap();

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

// -------- CONSOLA -------- //

void crearRestaurante(char**);
void crearReceta(char**);


// -------- MANEJO BITMAP -------- //

int obtenerYEscribirProximoDisponible(char*);
void eliminarBit(int);

// -------- RECETA -------- //

t_rta_obt_rec* cs_lectura_to_receta(char*);

// -------- PEDIDO -------- //

t_rta_obt_ped* cs_lectura_to_pedido(char*);
int existePedido(int, char*);
void escribirInfoPedido(char*, int, char*);
int obtenerBlockInicialPedido(int, char*);
int obtenerTamanioPedido(int, char*);
e_estado_ped cs_string_to_est_ped(char*);
int estaEnEstado(char*, e_estado_ped);
char* leerPedido(uint32_t,char*);
char* obtenerPathPedido(int idPedido, char* nombreRestaurante);
char* leerPedido(uint32_t idPedido, char* nombreRestaurante);

// -------- PISAR BLOQUES -------- //

void pisarPedido(uint32_t idPedido, char* nombreRestaurante, char* nuevaEscritura);
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
t_rta_obt_rest* cs_lectura_to_restaurante(char*, uint32_t);
bool existeRestaurante(char*);
char* leerRestaurante(char*);

// -------- AUX -------- //

int existeDirectorio(char*, int);
int cantidadDeBloques(char**);
int tamanioDeBloque(char*);
void liberar_lista(char**);
int tamanioArchivo(char*);
char* obtenerPathAbsoluto(const char* pathRelativo, ...);
t_dictionary* cs_lectura_to_dictionary(char* lectura);

#endif /* FILESYSTEM_H_ */
