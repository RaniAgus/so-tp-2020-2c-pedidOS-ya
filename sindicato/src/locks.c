/*
 * locks.c
 *
 *  Created on: 5 dic. 2020
 *      Author: utnso
 */
#include "locks.h"


void crearDiccionarioLocks(){
	diccionarioLocks = dictionary_create();
}



void lockearLectura(char* path){
	verificarExistencia(path);
	pthread_rwlock_rdlock(dictionary_get(diccionarioLocks,path));
}

void lockearEscritura(char* path){
	verificarExistencia(path);
	pthread_rwlock_wrlock(dictionary_get(diccionarioLocks,path));
}

void unlockear(char* path){
	pthread_rwlock_unlock(dictionary_get(diccionarioLocks,path));
}


void  verificarExistencia(char* path){
	if(!dictionary_has_key(diccionarioLocks,path)){
		pthread_rwlock_t* unLock = malloc(sizeof(pthread_rwlock_t));
		pthread_rwlock_init(unLock,NULL);
		dictionary_put(diccionarioLocks,path,unLock);
	}
}
