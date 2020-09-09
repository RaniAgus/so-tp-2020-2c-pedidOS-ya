#include "parser.h"

// <MSGTYPE>  ............<ARGS>............
#define CL_MIN_AMOUNT_ARGS  1
// NEW_POKEMON <POKEMON> <#POSX> <#POSY> <#CANT>
#define CL_NEW_POKEMON_ARGS 6

#define CL_CANT_ARGS_ERROR -1

#define CL_MSGTYPE_ARG  0	// <MSGTYPE>

typedef enum
{
	CL_NEW_POKEMON_NAME_ARG = 1, // <POKEMON>
	CL_NEW_POKEMON_POSX_ARG,	 // <#POSX>
	CL_NEW_POKEMON_POSY_ARG,	 // <#POSY>
	CL_NEW_POKEMON_CANT_ARG		 // <#CANT>
}cl_enum_new_args;

#define CHECK_NUMBER(val, arg) /*Tipo de error: argumento con un formato no válido (no es un número).*/\
    if(( val = cs_string_to_uint(argv[arg]) ) < 0) return arg

/**************************************** PARSER ********************************************/
static int cl_parse_msg_new_args(cl_parser_result* result, int argc, char* argv[]);

int client_parse_arguments(cl_parser_result* result, int argc, char* argv[])
{
	// Verifica que argc tenga el mínimo de argumentos necesarios
	if(argc < CL_MIN_AMOUNT_ARGS)
	{
		result->header.opcode  = OPCODE_UNKNOWN;
		result->header.msgtype = MSGTYPE_UNKNOWN;
		
		return CL_CANT_ARGS_ERROR;	//Tipo de error: argumentos insuficientes.
	}

	// Lee el código de operación
	result->header.opcode = OPCODE_SOLICITUD;

	// Lee el tipo de mensaje
	result->header.msgtype = (int8_t)cs_string_to_enum(argv[CL_MSGTYPE_ARG],
											   			cs_enum_msgtype_to_str);

	CS_LOG_TRACE("Se leyó el tipo de mensaje: %s",
					 cs_enum_opcode_to_str(result->header.msgtype));

	// Parsea según el tipo de mensaje
	switch(result->header.msgtype)
	{
	case NEW_POKEMON:
		return cl_parse_msg_new_args(result, argc, argv);
	default:
		return CL_MSGTYPE_ARG; //TODO: Ver valor de retorno!!!
	}

}

static int cl_parse_msg_new_args(cl_parser_result* result, int argc, char* argv[])
{
	// Verifica que argc tenga el mínimo de argumentos necesarios
	if(argc != CL_NEW_POKEMON_ARGS)	return CL_CANT_ARGS_ERROR; // Tipo de error: argumentos insuficientes.

	// Verifica que los valores sean numéricos
	int posx, posy, cant;
	CHECK_NUMBER(posx, CL_NEW_POKEMON_POSX_ARG);
	CHECK_NUMBER(posy, CL_NEW_POKEMON_POSY_ARG);
	CHECK_NUMBER(cant, CL_NEW_POKEMON_CANT_ARG);

	// Crea el mensaje (para el ejemplo, el msg_id está sin configurar)
	result->msg = (void*)cs_create_msg_new(
			argv[CL_NEW_POKEMON_NAME_ARG],
			(uint32_t)posx,
			(uint32_t)posy,
			(uint32_t)cant
	);

	return CL_SUCCESS;
}

/****************************** Mostrar error por pantalla ***********************************/

static void client_append_opcode_to_error(char** str_err_ptr, e_opcode opcode);
static void client_append_msg_to_error(char** str_err_ptr, e_msgtype msgtype);

void client_print_parser_error(int status, cl_parser_result result)
{
	char* err_str = string_new();
	switch(status)
	{
		//Tipo de error: faltan argumentos.
		case CL_CANT_ARGS_ERROR:
			string_append_with_format(&err_str, "Faltan argumentos.\n"
												"Se esperaba: ");
			client_append_opcode_to_error(&err_str, result.header.opcode);
			client_append_msg_to_error(&err_str, result.header.msgtype);
			break;
			//Tipo de error: opcode inválido.
		case CL_OPCODE_ARG:
			string_append_with_format(&err_str, "Código de operación no válido.\n"
												"Códigos de operación válidos: MENSAJE | ...");
			break;
			//Tipo de error: msgtype inválido.
		case CL_MSGTYPE_ARG:
			string_append_with_format(&err_str, "Tipo de mensaje no válido.\n"
												"Tipos de mensaje válidos: NEW_POKEMON | ...");
			break;
			//Tipo de error: argumentos con un formato no válido.
		default:
			string_append_with_format(&err_str, "Hay argumentos con un formato no válido.\n"
												"Se esperaba: ");
			client_append_opcode_to_error(&err_str, result.header.opcode);
			client_append_msg_to_error(&err_str, result.header.msgtype);
	}
	fprintf(stderr, "PARSER_ERROR (" __FILE__ ":%s:%d) -- %s\n", __func__ ,__LINE__, err_str);
}

static void client_append_opcode_to_error(char** str_err_ptr, e_opcode opcode)
{
	switch(opcode)
	{
		case OPCODE_UNKNOWN:
			string_append_with_format(str_err_ptr,"<OPCODE> ");
			break;
		case OPCODE_SOLICITUD:
			string_append_with_format(str_err_ptr, "MENSAJE ");
			break;
	}
}

static void client_append_msg_to_error(char** str_err_ptr, e_msgtype msgtype)
{
	switch(msgtype)
	{
		case MSGTYPE_UNKNOWN:
			string_append_with_format(str_err_ptr,"<MSGTYPE> <ARGS>");
			break;
		case NEW_POKEMON:
			string_append_with_format(str_err_ptr,"NEW_POKEMON <POKEMON> <#POSX> <#POSY> <#CANT>");
			break;
	}
}
