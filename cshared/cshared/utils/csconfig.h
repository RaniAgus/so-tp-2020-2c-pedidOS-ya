#ifndef UTILS_CONFIG_H_
#define UTILS_CONFIG_H_

#include "cserror.h"
#include <commons/config.h>

/**
* @NAME cs_config_init (versión de config_create de commons/config.h)
* @DESC Crea la estructura sconfig interna.
* @PARAMS
* 		path - path del archivo de configuracion
*/
e_status cs_config_init(const char* path) NON_NULL(1);

/**
* @NAME cs_config_delete (versión de config_destroy de commons/config.h)
* @DESC Destruye la estructura config interna.
*/
void         cs_config_delete(void);

/**
* @NAME cs_config_get_string (versión de config_get_string_value de commons/config.h)
* @DESC Retorna un string con el valor asociado a key.
*/
char*   cs_config_get_string(const char* key)      NON_NULL(1);

/**
* @NAME cs_config_get_int (versión de config_get_int_value de commons/config.h)
* @DESC Retorna un int con el valor asociado a key.
*/
int     cs_config_get_int(const char* key)         NON_NULL(1);

/**
* @NAME cs_config_get_long (versión de config_get_long_value de commons/config.h)
* @DESC Retorna un long con el valor asociado a key.
*/
long    cs_config_get_long(const char* key)        NON_NULL(1);

/**
* @NAME cs_config_get_double (versión de config_get_double_value de commons/config.h)
* @DESC Retorna un double con el valor asociado a key.
*/
double  cs_config_get_double(const char* key)      NON_NULL(1);

/**
* @NAME cs_config_get_array_value (versión de config_get_array_value de commons/config.h)
* @DESC Retorna un array con los valores asociados a la key especificada.
* En el archivo de configuracion un valor de este tipo debería ser representado
* de la siguiente forma [lista_valores_separados_por_coma]
* Ejemplo:
* VALORES=[1,2,3,4,5]
* El array que devuelve termina en NULL
*/
char**  cs_config_get_array_value(const char* key) NON_NULL(1);

/**
* @NAME cs_config_keys_amount (versión de config_key_amount de commons/config.h)
* @DESC Retorna la cantidad de keys.
*/
int  cs_config_keys_amount(void);

/**
* @NAME cs_config_set_value (versión de config_set_value de commons/config.h)
* @DESC Setea el valor en el archivo de config, a la key asociada.
*/
void cs_config_set_value(const char* key, const char* value) NON_NULL(1, 2);

/**
* @NAME cs_config_remove_key (versión de config_remove_key de commons/config.h)
* @DESC Remueve la clave y su valor asociado del archivo de config.
*/
void cs_config_remove_key(const char* key) NON_NULL(1);

/**
* @NAME cs_config_save (versión de config_save de commons/config.h)
* @DESC Reescribe el archivo de configuracion con los valores del config.
*/
void cs_config_save(void);

/**
* @NAME cs_config_save_in_file (versión de config_save_in_file de commons/config.h)
* @DESC Escribe un archivo de configuracion en el path indicado con los valores del config.
*/
e_status cs_config_save_in_file(const char* path) NON_NULL(1);

#endif /* UTILS_CONFIG_H_ */
