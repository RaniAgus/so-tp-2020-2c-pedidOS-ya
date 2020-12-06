#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cshared/cssend.h>
#include <cshared/csrecv.h>

#include <cspecs/cspec.h>

context(test_serializacion) {
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

	void* serializar_y_deserializar(t_header header, void* mensaje, e_module dest, int expected_size) {
		t_buffer* buffer = buffer_create();
		buffer_pack_msg(buffer, header, mensaje, dest);

		should_ptr(buffer) not be null;
		should_int(buffer->size) be equal to (expected_size);
		if(expected_size > 0) {
			should_ptr(buffer->stream) not be null;
		} else {
			should_ptr(buffer->stream) be null;
		}

		void* recibido = buffer_unpack_msg(header, buffer, -1);

		if(buffer->stream) free(buffer->stream);
		free(buffer);

		return recibido;
	}

	describe("Consultas") {

		before {
			cs_config_init("tests.config");
		} end

		after {
			cs_config_delete();
		} end

		it("Consulta sin parámetros") {
			t_header header = {OPCODE_CONSULTA, CONSULTAR_RESTAURANTES};
			t_consulta* enviado  = _cons_create(CONSULTAR_RESTAURANTES, NULL, 0, NULL, 0);
			t_consulta* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 0);

			should_ptr(recibido) not be null;
			should_ptr(recibido->comida) be null;
			should_int(recibido->cantidad) be equal to (0);
			should_ptr(recibido->restaurante) be null;
			should_int(recibido->pedido_id) be equal to (0);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Consulta con todos los parámetros") {
			t_header header = {OPCODE_CONSULTA, GUARDAR_PLATO};
			t_consulta* enviado  = _cons_create(GUARDAR_PLATO, "Bayern", 8, "Barcelona", 2);
			t_consulta* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 31);

			should_ptr(recibido) not be null;
			should_string(recibido->comida) be equal to ("Bayern");
			should_int(recibido->cantidad) be equal to (8);
			should_string(recibido->restaurante) be equal to ("Barcelona");
			should_int(recibido->pedido_id) be equal to (2);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Consulta handshake Cliente") {
			t_header header = {OPCODE_CONSULTA, HANDSHAKE_CLIENTE};
			t_handshake_cli* enviado  = cs_cons_handshake_cli_create();
			t_handshake_cli* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 20);

			should_ptr(recibido) not be null;
			should_string(recibido->nombre) be equal to ("Cliente1");
			should_int(recibido->posicion.x) be equal to (3);
			should_int(recibido->posicion.y) be equal to (2);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Consulta handshake Restaurante") {
			t_header header = {OPCODE_CONSULTA, HANDSHAKE_RESTAURANTE};
			t_pos posicion = {7, 8};
			t_handshake_res* enviado  = cs_cons_handshake_res_create(posicion);
			t_handshake_res* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 31);

			should_ptr(recibido) not be null;
			should_string(recibido->nombre) be equal to ("ElParrillon");
			should_int(recibido->posicion.x) be equal to (7);
			should_int(recibido->posicion.y) be equal to (8);
			should_ptr(recibido->ip) be null;
			should_string(recibido->puerto) be equal to ("4444");

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end
	} end

	describe ("Respuestas") {

		before {
			cs_config_init("tests.config");
		} end

		after {
			cs_config_delete();
		} end

		it("Consultar Restaurantes") {
			const char* restaurantes[] = { "Resto1", "Resto2", "Resto3", NULL };
			t_header header = {OPCODE_RESPUESTA_OK, CONSULTAR_RESTAURANTES};
			t_rta_cons_rest* enviado  = cs_rta_consultar_rest_create(string_get_string_as_array("[Resto1,Resto2,Resto3]"));
			t_rta_cons_rest* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 26);

			should_ptr(recibido) not be null;
			assert_array(recibido->restaurantes, restaurantes);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Seleccionar Restaurante") {
			t_header header = {OPCODE_RESPUESTA_OK, SELECCIONAR_RESTAURANTE};
			void* enviado  = NULL;
			void* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 0);

			should_ptr(recibido) be null;

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Obtener Restaurante") {
			t_pos posicion = { 2, 3 };
			const char* afinidades[] = {"Mollejas", "Choripan", NULL};

			t_header header = {OPCODE_RESPUESTA_OK, OBTENER_RESTAURANTE};
			t_rta_obt_rest* enviado = cs_rta_obtener_rest_create(4, "[Mollejas,Choripan]",
					"[AsadoCompleto,Choripan,Mollejas]", "[300,50,250]", posicion, 5, 10);
			t_rta_obt_rest* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 100);

			should_ptr(recibido) not be null;
			should_int(recibido->cant_cocineros) be equal to (4);
			assert_array(recibido->afinidades, afinidades);
			assert_comida_in_menu(recibido->menu, 0, "AsadoCompleto", 300);
			assert_comida_in_menu(recibido->menu, 1, "Choripan"     , 50 );
			assert_comida_in_menu(recibido->menu, 2, "Mollejas"     , 250);
			should_int(recibido->pos_restaurante.x) be equal to (2);
			should_int(recibido->pos_restaurante.y) be equal to (3);
			should_int(recibido->cant_hornos) be equal to (5);
			should_int(recibido->cant_pedidos) be equal to (10);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Consultar Platos") {
			const char* platos[] = { "Plato1", "Plato2", "Plato3", NULL };
			t_header header = {OPCODE_RESPUESTA_OK, CONSULTAR_PLATOS};
			t_rta_cons_pl* enviado  = cs_rta_consultar_pl_create("[Plato1,Plato2,Plato3]");
			t_rta_cons_pl* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 26);

			should_ptr(recibido) not be null;
			assert_array(recibido->comidas, platos);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Crear Pedido") {
			t_header header = {OPCODE_RESPUESTA_OK, CREAR_PEDIDO};
			t_rta_crear_ped* enviado = cs_rta_crear_ped_create(24);
			t_rta_crear_ped* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 4);

			should_ptr(recibido) not be null;
			should_int(recibido->pedido_id) be equal to (24);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Guardar Pedido") {
			t_header header = {OPCODE_RESPUESTA_OK, GUARDAR_PEDIDO};
			void* enviado  = NULL;
			void* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 0);

			should_ptr(recibido) be null;

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Aniadir Plato") {
			t_header header = {OPCODE_RESPUESTA_OK, ANIADIR_PLATO};
			void* enviado  = NULL;
			void* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 0);

			should_ptr(recibido) be null;

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Guardar Plato") {
			t_header header = {OPCODE_RESPUESTA_OK, GUARDAR_PLATO};
			void* enviado  = NULL;
			void* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 0);

			should_ptr(recibido) be null;

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Confirmar Pedido") {
			t_header header = {OPCODE_RESPUESTA_OK, CONFIRMAR_PEDIDO};
			void* enviado  = NULL;
			void* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 0);

			should_ptr(recibido) be null;

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Plato Listo") {
			t_header header = {OPCODE_RESPUESTA_OK, PLATO_LISTO};
			void* enviado  = NULL;
			void* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 0);

			should_ptr(recibido) be null;

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Consultar Pedido") {
			t_header header = {OPCODE_RESPUESTA_OK, CONSULTAR_PEDIDO};
			t_rta_cons_ped* enviado  = cs_rta_consultar_ped_create("ElParrillon", PEDIDO_CONFIRMADO, "[Choripan,AsadoCompleto]", "[2,1]", "[4,1]");
			t_rta_cons_ped* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 65);

			should_ptr(recibido) not be null;
			should_string(recibido->restaurante) be equal to ("ElParrillon");
			should_int(recibido->estado_pedido) be equal to (PEDIDO_CONFIRMADO);
			assert_plato_in_list(recibido->platos_y_estados, 0, "Choripan", 2, 4);
			assert_plato_in_list(recibido->platos_y_estados, 1, "AsadoCompleto", 1, 1);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Obtener Pedido") {
			t_header header = {OPCODE_RESPUESTA_OK, OBTENER_PEDIDO};
			t_rta_obt_ped* enviado  = cs_rta_obtener_ped_create(PEDIDO_CONFIRMADO, "[Choripan,AsadoCompleto]", "[2,1]", "[4,1]");
			t_rta_obt_ped* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 50);

			should_ptr(recibido) not be null;
			should_int(recibido->estado_pedido) be equal to (PEDIDO_CONFIRMADO);
			assert_plato_in_list(recibido->platos_y_estados, 0, "Choripan", 2, 4);
			assert_plato_in_list(recibido->platos_y_estados, 1, "AsadoCompleto", 1, 1);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Finalizar Pedido") {
			t_header header = {OPCODE_RESPUESTA_OK, FINALIZAR_PEDIDO};
			void* enviado  = NULL;
			void* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 0);

			should_ptr(recibido) be null;

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Terminar Pedido") {
			t_header header = {OPCODE_RESPUESTA_OK, TERMINAR_PEDIDO};
			void* enviado  = NULL;
			void* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 0);

			should_ptr(recibido) be null;

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Obtener Receta") {
			t_header header = {OPCODE_RESPUESTA_OK, OBTENER_RECETA};
			t_rta_obt_rec* enviado = cs_rta_obtener_receta_create("[Preparar,Servir]", "[2,1]");
			t_rta_obt_rec* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 34);

			should_ptr(recibido) not be null;
			assert_paso_in_receta(recibido->pasos_receta, 0, "Preparar", 2);
			assert_paso_in_receta(recibido->pasos_receta, 1, "Servir", 1);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Handshake Cliente") {
			t_header header = {OPCODE_RESPUESTA_OK, HANDSHAKE_CLIENTE};
			t_rta_handshake_cli* enviado = cs_rta_handshake_cli_create();
			t_rta_handshake_cli* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 1);

			should_ptr(recibido) not be null;
			should_int(recibido->modulo) be equal to (MODULO_APP);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

		it("Handshake Restaurante") {
			t_header header = {OPCODE_RESPUESTA_OK, HANDSHAKE_RESTAURANTE};
			void* enviado = NULL;
			void* recibido = serializar_y_deserializar(header, enviado, MODULO_DESCONOCIDO, 0);

			should_ptr(recibido) be null;

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

	} end

}
