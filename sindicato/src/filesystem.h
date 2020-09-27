#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "sindicato.h"

// -------- LEVANTAR FILESYSTEM -------- //

void crearDirectorioAFIP();
void crearMetadata(char*);
void crearFiles(char*);
void crearBlocks(char*);
void generarBitmap(char*, t_config*);

// -------- MANEJO BITMAP -------- //

int obtenerYEscribirProximoDisponible();
void eliminarBit(int);

#endif /* FILESYSTEM_H_ */
