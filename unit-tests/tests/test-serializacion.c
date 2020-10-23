#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cshared/cssend.h>
#include <cshared/csrecv.h>

#include <cspecs/cspec.h>

context(test_serializacion) {
	void* serializar_y_deserializar(void* mensaje, t_buffer*(*to_buffer)(void*), int size, t_header header) {
		t_buffer* buffer = to_buffer(mensaje);

		should_ptr(buffer) not be null;
		should_int(buffer->size) be equal to (size);
		if(size) {
			should_ptr(buffer->stream) not be null;
		} else {
			should_ptr(buffer->stream) be null;
		}

		void* recibido = cs_buffer_to_msg(header, buffer, -1);

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

		t_consulta* enviado;
		t_consulta* recibido;

		t_buffer* _consulta_to_buffer(t_consulta* consulta) {
			return cs_consulta_to_buffer(consulta, MODULO_COMANDA);
		}

		it("Consulta sin parámetros") {
			t_header header = {OPCODE_CONSULTA, CONSULTAR_RESTAURANTES};
			enviado  = _cons_create(CONSULTAR_RESTAURANTES, NULL, 0, NULL, 0);
			recibido = serializar_y_deserializar(enviado, (void*)_consulta_to_buffer, 0, header);

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
			enviado  = _cons_create(GUARDAR_PLATO, "Bayern", 8, "Barcelona", 2);
			recibido = serializar_y_deserializar(enviado, (void*)_consulta_to_buffer, 31, header);

			should_ptr(recibido) not be null;
			should_string(recibido->comida) be equal to ("Bayern");
			should_int(recibido->cantidad) be equal to (8);
			should_string(recibido->restaurante) be equal to ("Barcelona");
			should_int(recibido->pedido_id) be equal to (2);

			cs_msg_destroy(enviado, header.opcode, header.msgtype);
			cs_msg_destroy(recibido, header.opcode, header.msgtype);
		} end

/*		it("Consulta handshake Cliente") {

		} end

		it("Consulta handshake Restaurante") {

		} end
*/	} end
/*s
	describe ("Respuestas") {

		before {
			cs_config_init("tests.config");
		} end

		after {
			cs_config_delete();
		} end

		it("Consultar Restaurantes") {

		} end

		it("Seleccionar Restaurante") {

		} end

		it("Obtener Restaurante") {

		} end

		it("Consultar Platos") {

		} end

		it("Crear Pedido") {

		} end

		it("Guardar Pedido") {

		} end

		it("Aniadir Plato") {

		} end

		it("Guardar Plato") {

		} end

		it("Confirmar Pedido") {

		} end

		it("Plato Listo") {

		} end

		it("Consultar Pedido") {

		} end

		it("Obtener Pedido") {

		} end

		it("Finalizar Pedido") {

		} end

		it("Terminar Pedido") {

		} end

		it("Obtener Receta") {

		} end

		it("Handshake Cliente") {

		} end

		it("Handshake Restaurante") {

		} end

	} end
*/
}
