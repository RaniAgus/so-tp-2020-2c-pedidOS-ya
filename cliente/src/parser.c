#include "parser.h"

// <MSGTYPE>  ............<ARGS>............
#define CL_MIN_AMOUNT_ARGS  1
#define CL_MSGTYPE_ARG      0

/**************************************** PARSER ********************************************/

cl_parser_status client_parse_arguments(cl_parser_result* result, int argc, char* argv[])
{
	// Verifica que argc tenga el mínimo de argumentos necesarios
	if(argc < CL_MIN_AMOUNT_ARGS)
	{
		result->header.opcode  = OPCODE_UNKNOWN;
		result->header.msgtype = MSGTYPE_UNKNOWN;
		
		return CL_CANT_ARGS_ERROR;	//Tipo de error: argumentos insuficientes.
	}

	// El cliente solo envía solicitudes
	result->header.opcode = OPCODE_SOLICITUD;

	// Lee el tipo de mensaje
	result->header.msgtype = (int8_t)cs_string_to_enum(argv[CL_MSGTYPE_ARG],
											   			cs_enum_msgtype_to_str);

	CS_LOG_TRACE("Se leyó el tipo de mensaje: %s",
					 cs_enum_msgtype_to_str(result->header.msgtype));

	if(result->header.msgtype == MSGTYPE_UNKNOWN) return CL_MSGTYPE_ARG_ERROR;

	char *comida = string_new(), *restaurante = string_new();
	int cantidad = 0, pedido_id = 0;

	int arg = CL_MSGTYPE_ARG + 1;
	if(cs_sol_has_argument(result->header.msgtype, SOL_ARG_COMIDA))
	{
		if(arg == argc)
		{
			free(comida), free(restaurante);
			return CL_CANT_ARGS_ERROR;
		}

		CS_LOG_TRACE("<COMIDA> = %s", argv[arg]);

		string_append(&comida, argv[arg]);
		arg++;
	}

	if(cs_sol_has_argument(result->header.msgtype, SOL_ARG_CANTIDAD))
	{
		if(arg == argc)
		{
			free(comida), free(restaurante);
			return CL_CANT_ARGS_ERROR;
		}

		CS_LOG_TRACE("<#CANT> = %s", argv[arg]);

		cantidad = cs_string_to_uint(argv[arg]);
		if(cantidad < 0)
		{
			free(comida), free(restaurante);
			return CL_ARGS_ERROR;
		}
		arg++;
	}

	if(cs_sol_has_argument(result->header.msgtype, SOL_ARG_RESTAURANTE))
	{
		if(arg == argc)
		{
			free(comida), free(restaurante);
			return CL_CANT_ARGS_ERROR;
		}

		CS_LOG_TRACE("<RESTAURANTE> = %s", argv[arg]);

		string_append(&restaurante, argv[arg]);
		arg++;
	}

	if(cs_sol_has_argument(result->header.msgtype, SOL_ARG_PEDIDO_ID))
	{
		if(arg == argc)
		{
			free(comida), free(restaurante);
			return CL_CANT_ARGS_ERROR;
		}

		CS_LOG_TRACE("<#ID_PEDIDO> = %s", argv[arg]);

		pedido_id = cs_string_to_uint(argv[arg]);
		if(cantidad < 0)
		{
			free(comida), free(restaurante);
			return CL_ARGS_ERROR;
		}
		arg++;
	}

	for(; arg < argc; arg++)
	{
		CS_LOG_WARNING("WARNING - Argumento sin parsear: %s\n", argv[arg]);
	}

	result->msg = _sol_create(result->header.msgtype, comida, cantidad, restaurante, pedido_id);

	free(comida);
	free(restaurante);

	return CL_SUCCESS;

}

/****************************** Mostrar error por pantalla ***********************************/

static void client_append_msg_to_error(char** str_err_ptr, e_msgtype msgtype);

void client_print_parser_error(cl_parser_status status, cl_parser_result result)
{
	char* err_str = string_new();
	switch(status)
	{
		//Tipo de error: faltan argumentos.
		case CL_CANT_ARGS_ERROR:
			string_append_with_format(&err_str, "Faltan argumentos.\n"
												"Se esperaba: ");
			client_append_msg_to_error(&err_str, result.header.msgtype);
			break;
		//Tipo de error: msgtype inválido.
		case CL_MSGTYPE_ARG_ERROR:
			string_append_with_format(&err_str, "Tipo de mensaje no válido.\n"
												"Tipos de mensaje válidos: TIPO_DE_MENSAJE_1 | ...");
			break;
		//Tipo de error: argumentos con un formato no válido.
		default:
			string_append_with_format(&err_str, "Hay argumentos con un formato no válido.\n"
												"Se esperaba: ");
			client_append_msg_to_error(&err_str, result.header.msgtype);
	}
	fprintf(stderr, "PARSER_ERROR (" __FILE__ ":%s:%d) -- %s\n", __func__ ,__LINE__, err_str);
	free(err_str);
}


static void client_append_msg_to_error(char** str_err_ptr, e_msgtype msgtype)
{
	if(msgtype == MSGTYPE_UNKNOWN)
	{
		string_append(str_err_ptr,"<MSGTYPE> <ARGS>");
	} else
	{
		string_append(str_err_ptr, (char*)cs_enum_msgtype_to_str(msgtype));

		if(cs_sol_has_argument(msgtype, SOL_ARG_COMIDA))
		{
			string_append(str_err_ptr, " <PLATO>");
		}

		if(cs_sol_has_argument(msgtype, SOL_ARG_CANTIDAD))
		{
			string_append(str_err_ptr, " <#CANT>");
		}

		if(cs_sol_has_argument(msgtype, SOL_ARG_RESTAURANTE))
		{
			string_append(str_err_ptr, " <RESTAURANTE>");
		}

		if(cs_sol_has_argument(msgtype, SOL_ARG_PEDIDO_ID))
		{
			string_append(str_err_ptr, " <#ID_PEDIDO>");
		}
	}
}
