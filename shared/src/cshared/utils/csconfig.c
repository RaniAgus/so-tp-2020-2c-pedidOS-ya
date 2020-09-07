#include "csconfig.h"

static t_config* CS_CONFIG_INTERNAL = NULL;

e_status cs_config_init(const char* path)
{
    CS_CONFIG_INTERNAL = config_create((char*)path);

    return (CS_CONFIG_INTERNAL) ? STATUS_SUCCESS : ({cs_set_local_err(errno); STATUS_CONFIG_ERROR;});
}

void cs_config_delete(void)
{
    config_destroy(CS_CONFIG_INTERNAL);
}

char* cs_config_get_string(const char* key)
{
    return config_get_string_value(CS_CONFIG_INTERNAL, (char*)key);
}

int cs_config_get_int(const char* key)
{
    return config_get_int_value(CS_CONFIG_INTERNAL, (char*)key);
}

long cs_config_get_long(const char* key)
{
    return config_get_long_value(CS_CONFIG_INTERNAL, (char*)key);
}

double  cs_config_get_double(const char* key)
{
    return config_get_double_value(CS_CONFIG_INTERNAL, (char*)key);
}

char** cs_config_get_array_value(const char* key)
{
    return config_get_array_value(CS_CONFIG_INTERNAL, (char*)key);
}

int  cs_config_keys_amount(void)
{
    return config_keys_amount(CS_CONFIG_INTERNAL);
}

void cs_config_set_value(const char* key, const char* value)
{
    config_set_value(CS_CONFIG_INTERNAL, (char*)key, (char*)value);
}

void cs_config_remove_key(const char* key)
{
    config_remove_key(CS_CONFIG_INTERNAL, (char*)key);
}

void cs_config_save(void)
{
    config_save(CS_CONFIG_INTERNAL);
}

e_status cs_config_save_in_file(const char* path)
{
    return (config_save_in_file(CS_CONFIG_INTERNAL, (char*)path) != -1) ?
        STATUS_SUCCESS:
		({cs_set_local_err(errno); STATUS_CONFIG_ERROR;});
}
