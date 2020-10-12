#ifndef SHARED_CSHARED_H
#define SHARED_CSHARED_H

#include "utils/cscore.h"
#include "utils/cserror.h"
#include "utils/csconfig.h"
#include "utils/cslog.h"
#include "utils/csstring.h"
#include "utils/csconsole.h"
#include "utils/cstemporal.h"
#include "utils/cssignal.h"
#include "csstructs.h"
#include "csmsg.h"
#include "csmsgtostring.h"
#include "csconn.h"
#include "csconncreate.h"
#include "cssend.h"
#include "csrecv.h"

/**
* @NAME cs_parse_argument
* @DESC
*/
void cs_parse_argument(char* arg);

/**
* @NAME cs_module_init
* @DESC
*/
void cs_module_init(const char* config_file_path, const char* log_file_key, const char* module_name);

/**
* @NAME cs_module_close
* @DESC
*/
void cs_module_close(void);

#endif //SHARED_CSHARED_H
