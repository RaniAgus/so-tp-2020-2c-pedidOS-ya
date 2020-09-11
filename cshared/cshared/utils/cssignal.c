#include "cssignal.h"

e_status cs_signal_change_action(int signal, void (*new_action)(int), t_sigaction* old_action_ptr)
{
	t_sigaction sigint_action;
	memset(&sigint_action, 0, sizeof(sigint_action));

	sigint_action.sa_handler = new_action;

	if(sigaction(signal, &sigint_action, old_action_ptr) == -1)
	{
		cs_set_local_err(errno);
		return STATUS_SIGACTION_ERROR;
	}

	return STATUS_SUCCESS;
}