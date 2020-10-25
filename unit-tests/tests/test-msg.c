#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cshared/csmsgtypes.h>
#include <cshared/csmsgtostring.h>

#include <cspecs/cspec.h>

context(test_messages) {
	void assert_array(char** array, const char* expected[]) {
		int i = 0;
		void _assert_strings(char* line) {
			should_string(line) be equal to ((char*)expected[i]);
			i++;
		}
		string_iterate_lines(array, _assert_strings);
	}

	void assert_paso_in_receta(t_list* receta, int index, char* paso, int tiempo){
		t_paso_receta* paso_receta = list_get(receta, index);

		should_ptr(paso_receta) not be null;
		should_string(paso_receta->paso) be equal to (paso);
		should_int(paso_receta->tiempo) be equal to (tiempo);
	}

	void assert_plato_in_list(t_list* platos, int index, char* comida, int listos, int totales){
		t_plato* plato = list_get(platos, index);

		should_ptr(plato) not be null;
		should_string(plato->comida) be equal to (comida);
		should_int(plato->cant_lista) be equal to (listos);
		should_int(plato->cant_total) be equal to (totales);
	}

	void assert_comida_in_menu(t_list* comidas, int index, char* comida, int precio) {
		t_comida_menu* comida_menu = list_get(comidas, index);

		should_ptr(comida_menu) not be null;
		should_string(comida_menu->comida) be equal to (comida);
		should_int(comida_menu->precio) be equal to (precio);
	}

	char* msg_to_str;

	describe("Consultar Restaurantes") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_consultar_rest_create();

			should_ptr(consulta) not be null;

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, CONSULTAR_RESTAURANTES);
			should_string(msg_to_str) be equal to ("CONSULTAR_RESTAURANTES");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, CONSULTAR_RESTAURANTES);
			free(msg_to_str);
		} end

		it("Respuesta") {
			const char* restaurantes[] = {"Resto1", "Resto2", "Resto3", NULL};
			t_rta_cons_rest* respuesta = cs_rta_consultar_rest_create(string_get_string_as_array("[Resto1,Resto2,Resto3]"));

			should_ptr(respuesta) not be null;
			assert_array(respuesta->restaurantes, restaurantes);

			msg_to_str = cs_msg_to_str(respuesta, OPCODE_RESPUESTA_OK, CONSULTAR_RESTAURANTES);
			should_string(msg_to_str) be equal to ("CONSULTAR_RESTAURANTES {RESTAURANTES: [Resto1,Resto2,Resto3]}");
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, CONSULTAR_RESTAURANTES);
			free(msg_to_str);
		} end
	} end

	describe("Seleccionar Restaurante") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_seleccionar_rest_create("Resto");

			should_ptr(consulta) not be null;
			should_string(consulta->restaurante) be equal to ("Resto");

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, SELECCIONAR_RESTAURANTE);
			should_string(msg_to_str) be equal to ("SELECCIONAR_RESTAURANTE {RESTAURANTE: Resto}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, SELECCIONAR_RESTAURANTE);
			free(msg_to_str);
		} end

		it("Respuesta") {
			msg_to_str = cs_msg_to_str(NULL, OPCODE_RESPUESTA_OK, SELECCIONAR_RESTAURANTE);
			should_string(msg_to_str) be equal to ("SELECCIONAR_RESTAURANTE {RESULTADO: OK}");
			cs_msg_destroy(NULL, OPCODE_RESPUESTA_OK, SELECCIONAR_RESTAURANTE);
			free(msg_to_str);
		} end
	} end

	describe("Obtener Restaurante") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_obtener_rest_create("Resto");

			should_ptr(consulta) not be null;
			should_string(consulta->restaurante) be equal to ("Resto");

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, OBTENER_RESTAURANTE);
			should_string(msg_to_str) be equal to ("OBTENER_RESTAURANTE {RESTAURANTE: Resto}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, OBTENER_RESTAURANTE);
			free(msg_to_str);
		} end

		it("Respuesta") {
			t_pos posicion = { 2, 3 };
			const char* afinidades[] = {"Mollejas", "Choripan", NULL};

			t_rta_obt_rest* respuesta = cs_rta_obtener_rest_create(4, "[Mollejas,Choripan]",
					"[AsadoCompleto,Choripan,Mollejas]", "[300,50,250]", posicion, 5, 10);

			should_ptr(respuesta) not be null;
			should_int(respuesta->cant_cocineros) be equal to (4);
			assert_array(respuesta->afinidades, afinidades);
			assert_comida_in_menu(respuesta->menu, 0, "AsadoCompleto", 300);
			assert_comida_in_menu(respuesta->menu, 1, "Choripan"     , 50 );
			assert_comida_in_menu(respuesta->menu, 2, "Mollejas"     , 250);
			should_int(respuesta->pos_restaurante.x) be equal to (2);
			should_int(respuesta->pos_restaurante.y) be equal to (3);
			should_int(respuesta->cant_hornos) be equal to (5);
			should_int(respuesta->cant_pedidos) be equal to (10);

			msg_to_str = cs_msg_to_str(respuesta, OPCODE_RESPUESTA_OK, OBTENER_RESTAURANTE);
			should_string(msg_to_str) be equal to ("OBTENER_RESTAURANTE "
					"{COCINEROS: [#1:Mollejas,#2:Choripan,#3:Ninguna,#4:Ninguna]} {POSX: 2} {POSY: 3} "
					"{MENÚ: [AsadoCompleto($300),Choripan($50),Mollejas($250)]} "
					"{CANT_HORNOS: 5} {CANT_PEDIDOS: 10}");

			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, OBTENER_RESTAURANTE);
			free(msg_to_str);
		} end

	} end

	describe("Consultar Platos") {
		it("Consulta (App y Restaurante)") {
			t_consulta* consulta = cs_msg_consultar_pl_create();

			should_ptr(consulta) not be null;

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, CONSULTAR_PLATOS);
			should_string(msg_to_str) be equal to ("CONSULTAR_PLATOS");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, CONSULTAR_PLATOS);
			free(msg_to_str);
		} end

		it("Consulta (Sindicato)") {
			t_consulta* consulta = cs_msg_consultar_pl_rest_create("Resto");

			should_ptr(consulta) not be null;
			should_string(consulta->restaurante) be equal to ("Resto");

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, CONSULTAR_PLATOS);
			should_string(msg_to_str) be equal to ("CONSULTAR_PLATOS {RESTAURANTE: Resto}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, CONSULTAR_PLATOS);
			free(msg_to_str);
		} end

		it("Respuesta") {
			const char* comidas[] = {"AsadoCompleto", "Choripan", "Mollejas", NULL};
			t_rta_cons_pl* respuesta = cs_rta_consultar_pl_create("[AsadoCompleto,Choripan,Mollejas]");

			should_ptr(respuesta) not be null;
			assert_array(respuesta->comidas, comidas);

			msg_to_str = cs_msg_to_str(respuesta, OPCODE_RESPUESTA_OK, CONSULTAR_PLATOS);
			should_string(msg_to_str) be equal to ("CONSULTAR_PLATOS {PLATOS: [AsadoCompleto,Choripan,Mollejas]}");
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, CONSULTAR_PLATOS);
			free(msg_to_str);
		} end
	} end

	describe("Crear Pedido") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_crear_ped_create();

			should_ptr(consulta) not be null;

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, CREAR_PEDIDO);
			should_string(msg_to_str) be equal to ("CREAR_PEDIDO");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, CREAR_PEDIDO);
			free(msg_to_str);
		} end

		it("Respuesta") {
			t_rta_crear_ped* respuesta = cs_rta_crear_ped_create(35);

			should_ptr(respuesta) not be null;
			should_int (respuesta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(respuesta, OPCODE_RESPUESTA_OK, CREAR_PEDIDO);
			should_string(msg_to_str) be equal to ("CREAR_PEDIDO {ID_PEDIDO: 35}");
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, CREAR_PEDIDO);
			free(msg_to_str);
		} end
	} end

	describe("Guardar Pedido") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_guardar_ped_create("Resto", 35);

			should_ptr(consulta) not be null;
			should_string(consulta->restaurante) be equal to ("Resto");
			should_int(consulta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, GUARDAR_PEDIDO);
			should_string(msg_to_str) be equal to ("GUARDAR_PEDIDO {RESTAURANTE: Resto} {ID_PEDIDO: 35}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, GUARDAR_PEDIDO);
			free(msg_to_str);
		} end

		it("Respuesta") {
			msg_to_str = cs_msg_to_str(NULL, OPCODE_RESPUESTA_OK, GUARDAR_PEDIDO);
			should_string(msg_to_str) be equal to ("GUARDAR_PEDIDO {RESULTADO: OK}");
			cs_msg_destroy(NULL, OPCODE_RESPUESTA_OK, GUARDAR_PEDIDO);
			free(msg_to_str);
		} end
	} end

	describe("Aniadir Plato") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_aniadir_pl_create("Plato", 35);

			should_ptr(consulta) not be null;
			should_string(consulta->comida) be equal to ("Plato");
			should_int(consulta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, ANIADIR_PLATO);
			should_string(msg_to_str) be equal to ("ANIADIR_PLATO {COMIDA: Plato} {ID_PEDIDO: 35}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, ANIADIR_PLATO);
			free(msg_to_str);
		} end

		it("Respuesta") {
			msg_to_str = cs_msg_to_str(NULL, OPCODE_RESPUESTA_OK, ANIADIR_PLATO);
			should_string(msg_to_str) be equal to ("ANIADIR_PLATO {RESULTADO: OK}");
			cs_msg_destroy(NULL, OPCODE_RESPUESTA_OK, ANIADIR_PLATO);
			free(msg_to_str);
		} end
	} end

	describe("Guardar Plato") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_guardar_pl_create("Plato", 2, "Resto", 35);

			should_ptr(consulta) not be null;
			should_string(consulta->comida) be equal to ("Plato");
			should_int (consulta->cantidad) be equal to (2);
			should_string(consulta->restaurante) be equal to ("Resto");
			should_int (consulta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, GUARDAR_PLATO);
			should_string(msg_to_str) be equal to ("GUARDAR_PLATO {COMIDA: Plato} {CANT: 2} {RESTAURANTE: Resto} {ID_PEDIDO: 35}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, GUARDAR_PLATO);
			free(msg_to_str);
		} end

		it("Respuesta") {
			msg_to_str = cs_msg_to_str(NULL, OPCODE_RESPUESTA_OK, GUARDAR_PLATO);
			should_string(msg_to_str) be equal to ("GUARDAR_PLATO {RESULTADO: OK}");
			cs_msg_destroy(NULL, OPCODE_RESPUESTA_OK, GUARDAR_PLATO);
			free(msg_to_str);
		} end
	} end

	describe("Confirmar Pedido") {
		it("Consulta (App y Restaurante)") {
			t_consulta* consulta = cs_msg_confirmar_ped_create(35);

			should_ptr(consulta) not be null;
			should_int (consulta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, CREAR_PEDIDO);
			should_string(msg_to_str) be equal to ("CREAR_PEDIDO {ID_PEDIDO: 35}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, CREAR_PEDIDO);
			free(msg_to_str);
		} end

		it("Consulta (Comanda y Sindicato)") {
			t_consulta* consulta = cs_msg_confirmar_ped_rest_create("Resto",35);

			should_ptr(consulta) not be null;
			should_string (consulta->restaurante) be equal to ("Resto");
			should_int (consulta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, CREAR_PEDIDO);
			should_string(msg_to_str) be equal to ("CREAR_PEDIDO {RESTAURANTE: Resto} {ID_PEDIDO: 35}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, CREAR_PEDIDO);
			free(msg_to_str);
		} end

		it("Respuesta") {
			msg_to_str = cs_msg_to_str(NULL, OPCODE_RESPUESTA_OK, CONFIRMAR_PEDIDO);
			should_string(msg_to_str) be equal to ("CONFIRMAR_PEDIDO {RESULTADO: OK}");
			cs_msg_destroy(NULL, OPCODE_RESPUESTA_OK, CONFIRMAR_PEDIDO);
			free(msg_to_str);
		} end
	} end

	describe("Plato Listo") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_plato_listo_create("Plato", "Resto", 35);

			should_ptr(consulta) not be null;
			should_string(consulta->comida) be equal to ("Plato");
			should_string(consulta->restaurante) be equal to ("Resto");
			should_int (consulta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, PLATO_LISTO);
			should_string(msg_to_str) be equal to ("PLATO_LISTO {COMIDA: Plato} {RESTAURANTE: Resto} {ID_PEDIDO: 35}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, PLATO_LISTO);
			free(msg_to_str);
		} end

		it("Respuesta") {
			msg_to_str = cs_msg_to_str(NULL, OPCODE_RESPUESTA_OK, PLATO_LISTO);
			should_string(msg_to_str) be equal to ("PLATO_LISTO {RESULTADO: OK}");
			cs_msg_destroy(NULL, OPCODE_RESPUESTA_OK, PLATO_LISTO);
			free(msg_to_str);
		} end
	} end

	describe("Consultar Pedido") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_consultar_ped_create(35);

			should_ptr(consulta) not be null;
			should_int (consulta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, CONSULTAR_PEDIDO);
			should_string(msg_to_str) be equal to ("CONSULTAR_PEDIDO {ID_PEDIDO: 35}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, CONSULTAR_PEDIDO);
			free(msg_to_str);
		} end

		it("Respuesta") {
			t_rta_cons_ped* respuesta = cs_rta_consultar_ped_create("ElParrillon", PEDIDO_CONFIRMADO, "[AsadoCompleto,Choripan]", "[0,0]", "[1,2]");

			should_ptr(respuesta) not be null;
			should_string(respuesta->restaurante) be equal to ("ElParrillon");
			should_int(respuesta->estado_pedido) be equal to (PEDIDO_CONFIRMADO);
			assert_plato_in_list(respuesta->platos_y_estados, 0, "AsadoCompleto", 0, 1);
			assert_plato_in_list(respuesta->platos_y_estados, 1, "Choripan", 0, 2);

			msg_to_str = cs_msg_to_str(respuesta, OPCODE_RESPUESTA_OK, CONSULTAR_PEDIDO);
			should_string(msg_to_str) be equal to ("CONSULTAR_PEDIDO {RESTAURANTE: ElParrillon} {ESTADO_PEDIDO: Confirmado} "
					"{ESTADO_PLATOS: [AsadoCompleto(0/1),Choripan(0/2)]}");
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, CONSULTAR_PEDIDO);
			free(msg_to_str);
		} end

		it("Respuesta (pedido vacío)") {
			t_rta_cons_ped* respuesta = cs_rta_consultar_ped_create("ElParrillon", PEDIDO_PENDIENTE, "[]", "[]", "[]");

			should_ptr(respuesta) not be null;
			should_string(respuesta->restaurante) be equal to ("ElParrillon");
			should_int(respuesta->estado_pedido) be equal to (PEDIDO_PENDIENTE);
			should_bool(list_is_empty(respuesta->platos_y_estados)) be truthy;

			msg_to_str = cs_msg_to_str(respuesta, OPCODE_RESPUESTA_OK, CONSULTAR_PEDIDO);
			should_string(msg_to_str) be equal to ("CONSULTAR_PEDIDO {RESTAURANTE: ElParrillon} {ESTADO_PEDIDO: Pendiente} "
					"{ESTADO_PLATOS: []}");
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, CONSULTAR_PEDIDO);
			free(msg_to_str);
		} end
	} end

	describe("Obtener Pedido") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_obtener_ped_create("Resto", 35);

			should_ptr(consulta) not be null;
			should_string(consulta->restaurante) be equal to ("Resto");
			should_int(consulta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, OBTENER_PEDIDO);
			should_string(msg_to_str) be equal to ("OBTENER_PEDIDO {RESTAURANTE: Resto} {ID_PEDIDO: 35}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, OBTENER_PEDIDO);
			free(msg_to_str);
		} end

		it("Respuesta") {
			t_rta_obt_ped* respuesta = cs_rta_obtener_ped_create(PEDIDO_CONFIRMADO, "[AsadoCompleto,Choripan]", "[0,0]", "[1,2]");

			should_ptr(respuesta) not be null;
			should_int(respuesta->estado_pedido) be equal to (PEDIDO_CONFIRMADO);
			assert_plato_in_list(respuesta->platos_y_estados, 0, "AsadoCompleto", 0, 1);
			assert_plato_in_list(respuesta->platos_y_estados, 1, "Choripan", 0, 2);

			msg_to_str = cs_msg_to_str(respuesta, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
			should_string(msg_to_str) be equal to ("OBTENER_PEDIDO {ESTADO_PEDIDO: Confirmado} "
					"{ESTADO_PLATOS: [AsadoCompleto(0/1),Choripan(0/2)]}");
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
			free(msg_to_str);
		} end

		it("Respuesta (pedido vacío)") {
			t_rta_obt_ped* respuesta = cs_rta_obtener_ped_create(PEDIDO_PENDIENTE, "[]", "[]", "[]");

			should_ptr(respuesta) not be null;
			should_int(respuesta->estado_pedido) be equal to (PEDIDO_PENDIENTE);
			should_bool(list_is_empty(respuesta->platos_y_estados)) be truthy;

			msg_to_str = cs_msg_to_str(respuesta, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
			should_string(msg_to_str) be equal to ("OBTENER_PEDIDO {ESTADO_PEDIDO: Pendiente} "
					"{ESTADO_PLATOS: []}");
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
			free(msg_to_str);
		} end
	} end

	describe("Finalizar Pedido") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_fin_ped_create("Resto", 35);

			should_ptr(consulta) not be null;
			should_string(consulta->restaurante) be equal to ("Resto");
			should_int(consulta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, FINALIZAR_PEDIDO);
			should_string(msg_to_str) be equal to ("FINALIZAR_PEDIDO {RESTAURANTE: Resto} {ID_PEDIDO: 35}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, FINALIZAR_PEDIDO);
			free(msg_to_str);
		} end

		it("Respuesta") {
			msg_to_str = cs_msg_to_str(NULL, OPCODE_RESPUESTA_OK, FINALIZAR_PEDIDO);
			should_string(msg_to_str) be equal to ("FINALIZAR_PEDIDO {RESULTADO: OK}");
			cs_msg_destroy(NULL, OPCODE_RESPUESTA_OK, FINALIZAR_PEDIDO);
			free(msg_to_str);
		} end
	} end

	describe("Terminar Pedido") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_term_ped_create("Resto", 35);

			should_ptr(consulta) not be null;
			should_string(consulta->restaurante) be equal to ("Resto");
			should_int(consulta->pedido_id) be equal to (35);

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, TERMINAR_PEDIDO);
			should_string(msg_to_str) be equal to ("TERMINAR_PEDIDO {RESTAURANTE: Resto} {ID_PEDIDO: 35}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, TERMINAR_PEDIDO);
			free(msg_to_str);
		} end

		it("Respuesta") {
			msg_to_str = cs_msg_to_str(NULL, OPCODE_RESPUESTA_OK, TERMINAR_PEDIDO);
			should_string(msg_to_str) be equal to ("TERMINAR_PEDIDO {RESULTADO: OK}");
			cs_msg_destroy(NULL, OPCODE_RESPUESTA_OK, TERMINAR_PEDIDO);
			free(msg_to_str);
		} end
	} end

	describe("Obtener Receta") {
		it("Consulta") {
			t_consulta* consulta = cs_msg_rta_obtener_receta_create("Choripan");

			should_ptr(consulta) not be null;
			should_string(consulta->comida) be equal to ("Choripan");

			msg_to_str = cs_msg_to_str(consulta, OPCODE_CONSULTA, OBTENER_RECETA);
			should_string(msg_to_str) be equal to ("OBTENER_RECETA {COMIDA: Choripan}");
			cs_msg_destroy(consulta, OPCODE_CONSULTA, OBTENER_RECETA);
			free(msg_to_str);
		} end

		it("Respuesta") {
			t_rta_obt_rec* respuesta = cs_rta_obtener_receta_create("[Preparar,Servir]", "[2,1]");

			should_ptr(respuesta) not be null;
			assert_paso_in_receta(respuesta->pasos_receta, 0, "Preparar", 2);
			assert_paso_in_receta(respuesta->pasos_receta, 1, "Servir", 1);

			msg_to_str = cs_msg_to_str(respuesta, OPCODE_RESPUESTA_OK, OBTENER_RECETA);
			should_string(msg_to_str) be equal to ("OBTENER_RECETA {PASOS_RECETA: [Preparar(2),Servir(1)]}");
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, OBTENER_RECETA);
			free(msg_to_str);
		} end
	} end

	describe("Handshakes") {

		before {
			cs_config_init("tests.config");
		} end

		after {
			cs_config_delete();
		} end

		it("Handshake Cliente") {
			t_handshake_cli* handshake = cs_cons_handshake_cli_create();

			should_ptr(handshake) not be null;
			should_string(handshake->nombre) be equal to ("Cliente1");
			should_int(handshake->posicion.x) be equal to (3);
			should_int(handshake->posicion.y) be equal to (2);

			msg_to_str = cs_msg_to_str(handshake, OPCODE_CONSULTA, HANDSHAKE_CLIENTE);
			should_string(msg_to_str) be equal to ("HANDSHAKE_CLIENTE {NOMBRE: Cliente1} {POSX: 3} {POSY: 2}");
			cs_msg_destroy(handshake, OPCODE_CONSULTA, HANDSHAKE_CLIENTE);
			free(msg_to_str);
		} end

		it ("Respuesta handshake Cliente") {
			t_rta_handshake_cli* respuesta = cs_rta_handshake_cli_create();

			should_ptr(respuesta) not be null;
			should_int(respuesta->modulo) be equal to (MODULO_APP);

			msg_to_str = cs_msg_to_str(respuesta, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE);
			should_string(msg_to_str) be equal to ("HANDSHAKE_CLIENTE {MODULO: App}");
			cs_msg_destroy(respuesta, OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE);
			free(msg_to_str);
		} end

		it ("Handshake Restaurante") {
			t_pos posicion = {4, 5};
			t_handshake_res* handshake = cs_cons_handshake_res_create(posicion);

			should_ptr(handshake) not be null;
			should_string(handshake->nombre) be equal to ("ElParrillon");
			should_int(handshake->posicion.x) be equal to (4);
			should_int(handshake->posicion.y) be equal to (5);
			should_ptr(handshake->ip) be null;
			should_string(handshake->puerto) be equal to ("4444");

			msg_to_str = cs_msg_to_str(handshake, OPCODE_CONSULTA, HANDSHAKE_RESTAURANTE);
			should_string(msg_to_str) be equal to ("HANDSHAKE_RESTAURANTE {NOMBRE: ElParrillon} "
					"{POSX: 4} {POSY: 5} {PUERTO_ESCUCHA: 4444}");
			cs_msg_destroy(handshake, OPCODE_CONSULTA, HANDSHAKE_RESTAURANTE);
			free(msg_to_str);
		} end

	} end


}
