#ifndef CONN_RECV_H_
#define CONN_RECV_H_

#include "utils/cserror.h"
#include "csmsg.h"
#include "csconn.h"

/**
* @NAME cs_recv_msg
* @DESC Recibe un buffer desde la conexión indicada por parámetro, lo deserializa y
* aplica el closure al mensaje recibido. Retorna STATUS_SUCCESS en caso de que se haya
* recibido con éxito.
*/
e_status cs_recv_msg(t_sfd conn, void (*closure)(t_sfd, t_header, void*));

#endif /* CONNRECV_H_ */
