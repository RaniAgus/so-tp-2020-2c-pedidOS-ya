#ifndef SINDICATO_H_
#define SINDICATO_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/bitarray.h>
#include<readline/readline.h>
#include<cshared/cshared.h>
#include<semaphore.h>
#include<sys/mman.h>
#include <fcntl.h>

// ------------ SINCRO ------------ //

sem_t bitmapSem;

// ------------ ARCHIVOS ------------ //

t_log* logger;
t_config* config;

// ------------ CONFIG ------------ //

char* rutaLog;
char* magicNumber;
char* puntoMontaje;

int cantidadBloques;
int tamanioBloque;

// ------------ FUNCIONES ------------ //

void leerConfig();
t_log* iniciar_logger(void);


#endif /* SINDICATO_H_ */
