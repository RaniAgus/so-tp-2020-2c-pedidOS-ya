#include "parser.h"

// <MSGTYPE>  ............<ARGS>............
#define CL_MIN_AMOUNT_ARGS  1
#define CL_MSGTYPE_ARG      0

#define CHECK_NUMBER(val, arg)\
    if(( val = cs_string_to_uint(arg) ) < 0) return CL_ARGS_ERROR

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

	// El cliente solo envía mensajes
	result->header.opcode = OPCODE_MENSAJE;

	// Lee el tipo de mensaje
	result->header.msgtype = (int8_t)cs_string_to_enum(argv[CL_MSGTYPE_ARG],
											   			cs_enum_msgtype_to_str);

	CS_LOG_TRACE("Se leyó el tipo de mensaje: %s",
					 cs_enum_msgtype_to_str(result->header.msgtype));

	if(result->header.msgtype == MSGTYPE_UNKNOWN) return CL_MSGTYPE_ARG_ERROR;

	char* plato = string_new(), *restaurante = string_new();
	int cantidad = 0, pedido_id = 0;

	int arg = CL_MSGTYPE_ARG + 1;
	if(cs_msg_has_argument(result->header.msgtype, MSG_PLATO))
	{
		if(arg == argc) return CL_CANT_ARGS_ERROR;

		CS_LOG_TRACE("<PLATO> = %s", argv[arg]);

		string_append(&plato, argv[arg]);
		arg++;
	}

	if(cs_msg_has_argument(result->header.msgtype, MSG_CANTIDAD))
	{
		if(arg == argc) return CL_CANT_ARGS_ERROR;

		CS_LOG_TRACE("<#CANT> = %s", argv[arg]);

		CHECK_NUMBER(cantidad, argv[arg]);
		arg++;
	}

	if(cs_msg_has_argument(result->header.msgtype, MSG_RESTAURANTE))
	{
		if(arg == argc) return CL_CANT_ARGS_ERROR;

		CS_LOG_TRACE("<RESTAURANTE> = %s", argv[arg]);

		string_append(&restaurante, argv[arg]);
		arg++;
	}

	if(cs_msg_has_argument(result->header.msgtype, MSG_PEDIDO_ID))
	{
		if(arg == argc) return CL_CANT_ARGS_ERROR;

		CS_LOG_TRACE("<#ID_PEDIDO> = %s", argv[arg]);

		CHECK_NUMBER(pedido_id, argv[arg]);
		arg++;
	}

	result->msg = cs_msg_create(result->header.msgtype, plato, cantidad, restaurante, pedido_id);

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
}


static void client_append_msg_to_error(char** str_err_ptr, e_msgtype msgtype)
{
	if(msgtype == MSGTYPE_UNKNOWN)
	{
		string_append(str_err_ptr,"<MSGTYPE> <ARGS>");
	} else
	{
		string_append(str_err_ptr, (char*)cs_enum_msgtype_to_str(msgtype));

		if(cs_msg_has_argument(msgtype, MSG_PLATO))
		{
			string_append(str_err_ptr, " <PLATO>");
		}

		if(cs_msg_has_argument(msgtype, MSG_CANTIDAD))
		{
			string_append(str_err_ptr, " <#CANT>");
		}

		if(cs_msg_has_argument(msgtype, MSG_RESTAURANTE))
		{
			string_append(str_err_ptr, " <RESTAURANTE>");
		}

		if(cs_msg_has_argument(msgtype, MSG_PEDIDO_ID))
		{
			string_append(str_err_ptr, " <#ID_PEDIDO>");
		}
	}
}
