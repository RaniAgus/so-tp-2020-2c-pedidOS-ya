#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "sindicato.h"

void crearDirectorioAFIP();
void crearMetadata(char*);
void crearFiles(char*);
void crearBlocks(char*);
void generarBitmap(char*, t_config*);

#endif /* FILESYSTEM_H_ */
