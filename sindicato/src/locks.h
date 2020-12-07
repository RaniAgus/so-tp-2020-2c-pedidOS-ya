#ifndef LOCKS_H_
#define LOCKS_H_
#include<cshared/cshared.h>

t_dictionary* diccionarioLocks;

void crearDiccionarioLocks();
void lockearLectura(char*);
void lockearEscritura(char*);
void unlockear(char*);
void  verificarExistencia(char*);
char* obtenerPathLockPedido(t_consulta*);

#endif /* LOCKS_H_ */
