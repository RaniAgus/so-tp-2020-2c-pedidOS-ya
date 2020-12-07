#ifndef CLMESSAGES_H_
#define CLMESSAGES_H_

#include "clparser.h"

char*  prompt;
char*  serv_ip;
char*  serv_port;
t_sfd  serv_conn;
int8_t serv_module;

e_status client_send_handshake(t_sfd serv_conn, int8_t* module);
e_status client_recv_msg(t_sfd conn, int8_t* msg_type, int8_t* module);
void client_recv_msg_routine(void);
e_status client_send_msg(cl_parser_result* result);

#endif /* CLMESSAGES_H_ */
