#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cshared/csmsg.h>
#include <cshared/csmsgtostring.h>

#include <cspecs/cspec.h>

context(test_messages) {
	char* msg_to_str;

	after {
		free(msg_to_str);
	} end

	describe("Consultar Restaurantes") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Seleccionar Restaurante") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Obtener Restaurante") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Consultar Platos") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Crear Pedido") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Guardar Pedido") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Aniadir Plato") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Guardar Plato") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Confirmar Pedido") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Plato Listo") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Consultar Pedido") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Obtener Pedido") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Finalizar Pedido") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Terminar Pedido") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Obtener Receta") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Handshake Cliente") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

	describe("Handshake Restaurante") {
		it("Consulta") {

		} end

		it("Respuesta") {

		} end
	} end

}
