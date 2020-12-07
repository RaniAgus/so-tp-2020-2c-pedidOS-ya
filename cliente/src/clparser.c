#include "clparser.h"

#define PARSER_ASSERT(cond, errcode) do {\
	if(!(cond)) {\
		client_print_parser_error((errcode), result->msgtype, serv_module);\
		cs_msg_destroy(result->msg, OPCODE_CONSULTA, MSGTYPE_UNKNOWN);\
		free(result);\
		return NULL;\
	} } while(0)

static void client_print_parser_error(cl_parser_error status, e_msgtype msgtype, e_module serv_module);
static void client_append_msg_to_error(char** str_err_ptr, e_msgtype msgtype, e_module serv_module);

cl_parser_result* client_parse_arguments(int argc, char* argv[], e_module serv_module)
{
	int i = 0;
	cl_parser_result* result = malloc(sizeof(cl_parser_result));

	result->msg = _cons_create(NULL, 0, NULL, 0);
	result->msgtype = MSGTYPE_UNKNOWN;

	//Verifica que argc tenga el mínimo de argumentos necesarios
	PARSER_ASSERT(argc > 0, CL_ARGC_ERROR);

	//Lee el tipo de mensaje
	string_to_upper(argv[i]);
	result->msgtype = (int8_t)cs_string_to_enum(argv[i], cs_enum_msgtype_to_str);
	CS_LOG_TRACE("Se leyó el tipo de mensaje: %s", cs_enum_msgtype_to_str(result->msgtype));
	PARSER_ASSERT(result->msgtype > MSGTYPE_UNKNOWN && result->msgtype < HANDSHAKE_CLIENTE, CL_INVALID_MSGTYPE);
	i++;

	//Lee cada parámetro
	if(cs_cons_has_argument(result->msgtype, CONS_ARG_COMIDA, serv_module))
	{
		PARSER_ASSERT(i < argc, CL_ARGC_ERROR);
		CS_LOG_TRACE("{COMIDA: %s}", argv[i]);

		result->msg->comida = string_duplicate(argv[i]);
		i++;
	}
	if(cs_cons_has_argument(result->msgtype, CONS_ARG_CANTIDAD, serv_module))
	{
		PARSER_ASSERT(i < argc, CL_ARGC_ERROR);
		CS_LOG_TRACE("{CANTIDAD: %s}", argv[i]);
		PARSER_ASSERT(cs_string_to_uint(argv[i]) >= 0, CL_INVALID_ARG);

		result->msg->cantidad = cs_string_to_uint(argv[i]);
		i++;
	}
	if(cs_cons_has_argument(result->msgtype, CONS_ARG_RESTAURANTE, serv_module))
	{
		PARSER_ASSERT(i < argc, CL_ARGC_ERROR);
		CS_LOG_TRACE("{RESTAURANTE: %s}", argv[i]);

		result->msg->restaurante = string_duplicate(argv[i]);
		i++;
	}
	if(cs_cons_has_argument(result->msgtype, CONS_ARG_PEDIDO_ID, serv_module))
	{
		PARSER_ASSERT(i < argc, CL_ARGC_ERROR);
		CS_LOG_TRACE("{ID_PEDIDO: %s}", argv[i]);
		PARSER_ASSERT(cs_string_to_uint(argv[i]) >= 0, CL_INVALID_ARG);

		result->msg->pedido_id = cs_string_to_uint(argv[i]);
		i++;
	}

	for(; i < argc; i++) {
		CS_LOG_WARNING("Parámetro no leído: %s", argv[i]);
	}

	return result;
}

static void client_print_parser_error(cl_parser_error status, e_msgtype msgtype, e_module serv_module)
{
	char* err_str = string_new();
	switch(status)
	{
		//Tipo de error: msgtype inválido.
		case CL_INVALID_MSGTYPE:
			string_append(&err_str, "Tipo de mensaje no válido.");
			break;
		//Tipo de error: faltan argumentos.
		case CL_ARGC_ERROR:
			string_append(&err_str, "Faltan parámetros.\nSe esperaba: ");
			client_append_msg_to_error(&err_str, msgtype, serv_module);
			break;
		//Tipo de error: argumentos con un formato no válido.
		case CL_INVALID_ARG:
			string_append(&err_str, "Hay parámetros con un formato no válido.\nSe esperaba: ");
			client_append_msg_to_error(&err_str, msgtype, serv_module);
			break;
		default:
			string_append(&err_str, "Error desconocido.");
			break;
	}
	CS_LOG_ERROR("Error al parsear: %s", err_str);
	free(err_str);
}

static void client_append_msg_to_error(char** str_err_ptr, e_msgtype msgtype, e_module serv_module)
{
	string_append(str_err_ptr, (char*)cs_enum_msgtype_to_str(msgtype));

	if(cs_cons_has_argument(msgtype, CONS_ARG_COMIDA, serv_module))
		string_append(str_err_ptr, " [COMIDA]");

	if(cs_cons_has_argument(msgtype, CONS_ARG_CANTIDAD, serv_module))
		string_append(str_err_ptr, " [#CANTIDAD]");

	if(cs_cons_has_argument(msgtype, CONS_ARG_RESTAURANTE, serv_module))
		string_append(str_err_ptr, " [RESTAURANTE]");

	if(cs_cons_has_argument(msgtype, CONS_ARG_PEDIDO_ID, serv_module))
		string_append(str_err_ptr, " [#ID_PEDIDO]");
}
