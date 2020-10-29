#include "clparser.h"

/**************************************** PARSER ********************************************/

cl_parser_status client_parse_arguments(cl_parser_result* result, int argc, char* argv[], e_module serv_module)
{
	// Verifica que argc tenga el mínimo de argumentos necesarios
	if(argc < 1)
	{
		result->msgtype = MSGTYPE_UNKNOWN;

		return CL_CANT_ARGS_ERROR;	//Tipo de error: argumentos insuficientes.
	}

	// Lee el tipo de mensaje
	string_to_upper(argv[0]);
	result->msgtype = (int8_t)cs_string_to_enum(argv[0],
											   			cs_enum_msgtype_to_str);

	CS_LOG_TRACE(__FILE__":%s:%d -- Se leyó el tipo de mensaje: %s",
					  __func__, __LINE__, cs_enum_msgtype_to_str(result->msgtype));

	if(result->msgtype == MSGTYPE_UNKNOWN || result->msgtype == HANDSHAKE_CLIENTE || result->msgtype == HANDSHAKE_RESTAURANTE) return CL_MSGTYPE_ARG_ERROR;

	char *comida = NULL, *restaurante = NULL;
	int cantidad = 0, pedido_id = 0;

	int arg = 1;
	if(cs_cons_has_argument(result->msgtype, CONS_ARG_COMIDA, serv_module))
	{
		if(arg == argc)	return CL_CANT_ARGS_ERROR;

		CS_LOG_TRACE(__FILE__":%s:%d -- <COMIDA> = %s",  __func__, __LINE__, argv[arg]);

		comida = string_duplicate(argv[arg]);
		arg++;
	}

	if(cs_cons_has_argument(result->msgtype, CONS_ARG_CANTIDAD, serv_module))
	{
		if(arg == argc)
		{
			if(comida) free(comida);
			return CL_CANT_ARGS_ERROR;
		}

		CS_LOG_TRACE(__FILE__":%s:%d -- <#CANT> = %s", __func__, __LINE__, argv[arg]);

		cantidad = cs_string_to_uint(argv[arg]);
		if(cantidad < 0)
		{
			if(comida) free(comida);
			return CL_ARGS_ERROR;
		}
		arg++;
	}

	if(cs_cons_has_argument(result->msgtype, CONS_ARG_RESTAURANTE, serv_module))
	{
		if(arg == argc)
		{
			if(comida) free(comida);
			return CL_CANT_ARGS_ERROR;
		}

		CS_LOG_TRACE(__FILE__":%s:%d -- <RESTAURANTE> = %s", __func__, __LINE__, argv[arg]);

		restaurante = string_duplicate(argv[arg]);
		arg++;
	}

	if(cs_cons_has_argument(result->msgtype, CONS_ARG_PEDIDO_ID, serv_module))
	{
		if(arg == argc)
		{
			if(comida)      free(comida); 
			if(restaurante) free(restaurante);
			return CL_CANT_ARGS_ERROR;
		}

		CS_LOG_TRACE(__FILE__":%s:%d -- <#ID_PEDIDO> = %s", __func__, __LINE__, argv[arg]);

		pedido_id = cs_string_to_uint(argv[arg]);
		if(pedido_id < 0)
		{
			if(comida)      free(comida); 
			if(restaurante) free(restaurante);
			return CL_ARGS_ERROR;
		}
		arg++;
	}

	for(; arg < argc; arg++)
	{
		CS_LOG_WARNING("Argumento sin parsear: %s", argv[arg]);
	}

	result->msg = _cons_create(result->msgtype, comida, cantidad, restaurante, pedido_id);

	if(comida)      free(comida);
	if(restaurante) free(restaurante);

	return CL_SUCCESS;

}

/****************************** Mostrar error por pantalla ***********************************/

static void client_append_msg_to_error(char** str_err_ptr, e_msgtype msgtype, e_module serv_module);

void client_print_parser_error(cl_parser_status status, cl_parser_result result, e_module serv_module)
{
	char* err_str = string_new();
	switch(status)
	{
		//Tipo de error: faltan argumentos.
		case CL_CANT_ARGS_ERROR:
			string_append_with_format(&err_str, "Faltan argumentos.\n"
												"Se esperaba: ");
			client_append_msg_to_error(&err_str, result.msgtype, serv_module);
			break;
		//Tipo de error: msgtype inválido.
		case CL_MSGTYPE_ARG_ERROR:
			string_append_with_format(&err_str, "Tipo de mensaje no válido.");
			break;
		//Tipo de error: argumentos con un formato no válido.
		default:
			string_append_with_format(&err_str, "Hay argumentos con un formato no válido.\n"
												"Se esperaba: ");
			client_append_msg_to_error(&err_str, result.msgtype, serv_module);
	}
	fprintf(stderr, "[PARSER_ERROR]: %s\n", err_str);
	free(err_str);
}


static void client_append_msg_to_error(char** str_err_ptr, e_msgtype msgtype, e_module serv_module)
{
	if(msgtype == MSGTYPE_UNKNOWN)
	{
		string_append(str_err_ptr,"<MSGTYPE> <ARGS>");
	} else
	{
		string_append(str_err_ptr, (char*)cs_enum_msgtype_to_str(msgtype));

		if(cs_cons_has_argument(msgtype, CONS_ARG_COMIDA, serv_module))
		{
			string_append(str_err_ptr, " <PLATO>");
		}

		if(cs_cons_has_argument(msgtype, CONS_ARG_CANTIDAD, serv_module))
		{
			string_append(str_err_ptr, " <#CANT>");
		}

		if(cs_cons_has_argument(msgtype, CONS_ARG_RESTAURANTE, serv_module))
		{
			string_append(str_err_ptr, " <RESTAURANTE>");
		}

		if(cs_cons_has_argument(msgtype, CONS_ARG_PEDIDO_ID, serv_module))
		{
			string_append(str_err_ptr, " <#ID_PEDIDO>");
		}
	}
}
