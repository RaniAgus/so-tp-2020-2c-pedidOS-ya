#ifndef SHARED_CSSIGNAL_H
#define SHARED_CSSIGNAL_H

#include <signal.h>
#include <string.h>
#include "cserror.h"

typedef struct sigaction t_sigaction;

/**
* @NAME cs_signal_change_action
* @DESC Cambia la acción de la señal 'signal' por un llamado a la función 'new_action',
 * retornando la acción anterior por parámetro.
*/
e_status cs_signal_change_action(int signal, void (*new_action)(int), t_sigaction* old_action_ptr);

#endif //SHARED_CSSIGNAL_H
