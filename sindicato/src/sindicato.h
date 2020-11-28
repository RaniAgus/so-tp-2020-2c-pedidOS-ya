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
#include<sys/stat.h>
#include<fcntl.h>
#include<commons/memory.h>

// ------------ COMUNICACIONES ------------ //

char* miPuerto;
t_sfd escucha;

// ------------ SINCRO ------------ //

sem_t bitmapSem;
int tamanioReservado;

// ------------ ARCHIVOS ------------ //

#define MODULE_NAME		 "SINDICATO"
#define CONFIG_FILE_PATH "sindicato.config"
#define LOG_FILE_KEY	 "RUTA_LOG"


t_log* logger;
t_config* config;

// ------------ CONFIG ------------ //

char* rutaLog;
char* puntoMontaje;

int cantidadBloques;
int tamanioBloque;

// ------------ FUNCIONES ------------ //

void* atenderConsola();
void leerConfig();
void server_recv_msg(t_sfd*);
void server_log_and_send_reply(t_sfd, t_header, void*);
void server_send_rta_handshake(t_sfd);
void server_send_rta_ok_fail(e_msgtype, t_sfd, e_opcode);
void server_error_handler(e_status);

#endif /* SINDICATO_H_ */
