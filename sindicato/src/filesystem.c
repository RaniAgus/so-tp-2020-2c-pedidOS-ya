#include "filesystem.h"

// --------------------- LEVANTAR FILE SYSTEM --------------------- //

void crearDirectorioAFIP(){
	obtenerMetadata();
	char* pathBitmap = obtenerPathAbsoluto("Metadata/Bitmap.bin");

	if(fopen(pathBitmap, "r") == NULL){
		//CS_LOG_INFO("Genere el directorio %s", puntoMontaje);
		crearFiles();
		crearBlocks();
		generarBitmap();
	} else {
		CS_LOG_TRACE("Ya existe el Bitmap\n");
	}

	free(pathBitmap);
}

void obtenerMetadata(){
	char* pathMedadata = obtenerPathAbsoluto("Metadata/Metadata.AFIP");
	t_config* md = config_create(pathMedadata);
	if(md == NULL) {
		CS_LOG_ERROR("No se pudo obtener el archivo metadata desde: %s", pathMedadata);
		exit(-1);
	}
	tamanioBloque = config_get_int_value(md, "BLOCK_SIZE");
	cantidadBloques = config_get_int_value(md, "BLOCKS");

	config_destroy(md);
	free(pathMedadata);
}

void crearFiles(){
	char* pathFiles = obtenerPathAbsoluto("Files");
	char* pathRestaurantes = obtenerPathAbsoluto("Files/Restaurantes");
	char* pathRecetas = obtenerPathAbsoluto("Files/Recetas");

	mkdir(pathFiles, 0777);
	CS_LOG_INFO("Agrego directorio Files");
	mkdir(pathRestaurantes, 0777);
	CS_LOG_INFO("Agrego directorio Restaurantes");
	mkdir(pathRecetas, 0777);
	CS_LOG_INFO("Agrego directorio Recetas");

	free(pathFiles);
	free(pathRestaurantes);
	free(pathRecetas);
}

void crearBlocks(){
	char* pathBlocks = obtenerPathAbsoluto("Blocks");
	mkdir(pathBlocks, 0777);
	CS_LOG_INFO("Agrego el directorio Blocks");

	for(int i=1; i<=cantidadBloques; i++){
		limpiarBloque(i);
	}
	CS_LOG_INFO("Se agregaron %d archivos BLOCKS de tamaño %d", cantidadBloques, tamanioBloque);

	free(pathBlocks);
}

void generarBitmap(char* pathOrigin){
	char* pathBitmap = obtenerPathAbsoluto("Metadata/Bitmap.bin");
	void* punteroABitmap = calloc(cantidadBloques/8, 1);

	FILE* bitmapFile = fopen(pathBitmap, "wrb");
	fwrite(punteroABitmap, 1, cantidadBloques/8, bitmapFile);
	fclose(bitmapFile);

	free(punteroABitmap);
	free(pathBitmap);

	CS_LOG_INFO("Genere el bitmap");
}

// --------------------------------------------------------- //
// ----------------- MENSAJES COMPONENTES ------------------ //
// --------------------------------------------------------- //

t_rta_cons_pl* consultarPlatos(t_consulta* consulta){ // LISTO
	t_rta_cons_pl* respuesta = NULL;
	char* lectura = leerRestaurante(consulta->restaurante);
	if(lectura != NULL){
		char* platos = obtenerPlatos(lectura);
		respuesta = cs_rta_consultar_pl_create(platos);
		free(platos);
		free(lectura);
	}

	return respuesta;
}

e_opcode guardarPedido(t_consulta* consulta){ // LISTO
	e_opcode respuesta;
	if(existeRestaurante(consulta->restaurante)){
		if(existePedido(consulta->pedido_id, consulta->restaurante)){
			respuesta = OPCODE_RESPUESTA_FAIL;
			CS_LOG_ERROR("Ya existe el pedido %d", consulta->pedido_id);
		} else {
			char* escritura = string_from_format(
						"ESTADO_PEDIDO=Pendiente\n"
						"LISTA_PLATOS=[]\n"
						"CANTIDAD_PLATOS=[]\n"
						"CANTIDAD_LISTA=[]\n"
						"PRECIO_TOTAL=0\n"
			);
			char* infoPedido = escribirNuevoArchivo(escritura, "Restaurante %s Pedido %d", consulta->restaurante, consulta->pedido_id);
			if(infoPedido != NULL) {
				escribirInfoPedido(infoPedido, consulta->pedido_id, consulta->restaurante);
				respuesta = OPCODE_RESPUESTA_OK;
			} else {
				respuesta = OPCODE_RESPUESTA_FAIL;
			}
			free(escritura);
		}
	} else {
		respuesta = OPCODE_RESPUESTA_FAIL;
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	return respuesta;
}

e_opcode guardarPlato(t_consulta* consulta){ // LISTO
	e_opcode respuesta = OPCODE_RESPUESTA_FAIL;
	char* lectura = leerPedido(consulta->pedido_id, consulta->restaurante);
	if(lectura != NULL) {
		char* restaurante = leerRestaurante(consulta->restaurante);
		if(buscarPlatoEnRestaurante(restaurante, consulta)){
			if(estaEnEstado(lectura, PEDIDO_PENDIENTE)){
				char* nuevaEscritura;
				if(buscarPlatoEnPedido(lectura, consulta)){
					nuevaEscritura = agregarCantPlatos(lectura ,consulta);
				} else {
					nuevaEscritura = agregarPlato(lectura, consulta);
				}
				respuesta = pisarPedido(consulta->pedido_id, consulta->restaurante, nuevaEscritura);
				free(nuevaEscritura);
			} else {
				CS_LOG_ERROR("El pedido %d ya no se encuentra pendiente", consulta->pedido_id);
			}
		} else {
			CS_LOG_ERROR("El restaurante %s no dispone del plato %s en su menu", consulta->restaurante, consulta->comida);
		}
		free(restaurante);
		free(lectura);
	}
	return respuesta;
}

e_opcode confirmarPedido(t_consulta* consulta){ // LISTO
	e_opcode respuesta = OPCODE_RESPUESTA_FAIL;
	char* lectura = leerPedido(consulta->pedido_id, consulta->restaurante);
	if(lectura != NULL) {
		if(estaEnEstado(lectura, PEDIDO_PENDIENTE)){
			char* nuevaEscritura = cambiarEstadoPedidoA(lectura, consulta, PEDIDO_CONFIRMADO);
			respuesta = pisarPedido(consulta->pedido_id, consulta->restaurante, nuevaEscritura);
			free(nuevaEscritura);
		} else {
			CS_LOG_ERROR("El pedido %d ya no se encuentra pendiente", consulta->pedido_id);
		}
		free(lectura);
	}
	return respuesta;
}

t_rta_obt_ped* obtenerPedido(t_consulta* consulta){ // LISTO
	t_rta_obt_ped* respuesta = NULL;
	char* lectura = leerPedido(consulta->pedido_id, consulta->restaurante);
	if(lectura != NULL) {
		respuesta = cs_lectura_to_pedido(lectura);
		free(lectura);
	}

	return respuesta;
}

t_rta_obt_rest* obtenerRestaurante(t_consulta* consulta){  // LISTO
	t_rta_obt_rest* respuesta = NULL;
	char* lectura = leerRestaurante(consulta->restaurante);
	if(lectura != NULL){
		uint32_t cantPedidos = obtenerCantidadPedidos(consulta->restaurante);
		respuesta = cs_lectura_to_restaurante(lectura, cantPedidos);
		free(lectura);
	}
	return respuesta;
}

e_opcode platoListo(t_consulta* consulta){ //
	e_opcode respuesta = OPCODE_RESPUESTA_FAIL;
	char* lectura = leerPedido(consulta->pedido_id, consulta->restaurante);
	if(lectura != NULL) {
		if(estaEnEstado(lectura, PEDIDO_CONFIRMADO)){
			if(buscarPlatoEnPedido(lectura, consulta)){
				char* nuevaEscritura = agregarPlatoListo(lectura, consulta);
				if(nuevaEscritura != NULL) {
					respuesta = pisarPedido(consulta->pedido_id, consulta->restaurante, nuevaEscritura);
					free(nuevaEscritura);
				} else {
					CS_LOG_ERROR("Ya estaban listos todos los platos %s del pedido %d del restaurante %s", consulta->comida, consulta->pedido_id, consulta->restaurante);
				}
			} else {
				CS_LOG_ERROR("El pedido %d del restaurante %s no posee el plato %s", consulta->pedido_id, consulta->restaurante, consulta->comida);
			}
		} else {
			CS_LOG_ERROR("El pedido %d no esta en estado confirmado", consulta->pedido_id);
		}
		free(lectura);
	}
	return respuesta;
}

e_opcode terminarPedido(t_consulta* consulta){ //
	e_opcode respuesta = OPCODE_RESPUESTA_FAIL;
	char* lectura = leerPedido(consulta->pedido_id, consulta->restaurante);
	if(lectura != NULL) {
		if(estaEnEstado(lectura, PEDIDO_CONFIRMADO)){
			char* nuevaEscritura = cambiarEstadoPedidoA(lectura, consulta, PEDIDO_TERMINADO);
			respuesta = pisarPedido(consulta->pedido_id, consulta->restaurante, nuevaEscritura);
			free(nuevaEscritura);
		} else {
			CS_LOG_ERROR("El pedido %d tovadía no se ha confirmado", consulta->pedido_id);
		}
		free(lectura);
	}

	return respuesta;
}

t_rta_obt_rec* obtenerReceta(t_consulta* consulta){ // LISTO
	t_rta_obt_rec* respuesta = NULL;
	char* path = obtenerPathAbsoluto("Files/Recetas/%s.AFIP", consulta->comida);
	if(existeDirectorio(path, 0)){
		t_config* conf = config_create(path);
		int bloque = config_get_int_value(conf, "INITIAL_BLOCK");
		int size = config_get_int_value(conf, "SIZE");
		config_destroy(conf);
		char* lectura = leerBloques(bloque, size);
		respuesta = cs_lectura_to_receta(lectura);
		free(lectura);
	} else {
		CS_LOG_ERROR("No existe el Plato %s", consulta->comida);
	}
	free(path);
	return respuesta;
}

// --------------------------------------------------------- //
// ----------------- MENSAJES CONSOLA ------------------ //
// --------------------------------------------------------- //

void crearRestaurante(char** consulta){ // LISTO
	char* path = obtenerPathRestaurante(consulta[RES_NOMBRE]);
	existeDirectorio(path ,1);
	char* pathCarpeta = string_duplicate(path);
	CS_LOG_INFO("Se creo el restaurante %s", consulta[RES_NOMBRE]);
	
	string_append(&path, "/Info.AFIP");
	char* escritura = string_from_format(
			"CANTIDAD_COCINEROS=%s\n"
			"POSICION=%s\n"
			"AFINIDAD_COCINEROS=%s\n"
			"PLATOS=%s\n"
			"PRECIO_PLATOS=%s\n"
			"CANTIDAD_HORNOS=%s\n"
			, consulta[RES_CANT_COCINEROS]
			, consulta[RES_POSICION]
			, consulta[RES_AFINIDADES]
			, consulta[RES_PLATOS]
			, consulta[RES_PRECIOS]
			, consulta[RES_CANT_HORNOS]
	);

	char* infoRes = escribirNuevoArchivo(escritura, "Restaurante %s", consulta[RES_NOMBRE]);
	if(infoRes != NULL) {
		FILE* fd = fopen(path, "wt");
		fwrite(infoRes, strlen(infoRes), 1, fd);
		fclose(fd);
		CS_LOG_INFO("Se creo el archivo \"Info.AFIP\" para el restaurant %s", consulta[RES_NOMBRE]);

		free(infoRes);
	} else {
		remove(pathCarpeta);
	}
	free(pathCarpeta);
	free(path);
	free(escritura);
}

void crearReceta(char** consulta){ // LISTO
	char* path = obtenerPathAbsoluto("Files/Recetas/%s.AFIP", consulta[REC_NOMBRE]);
	char* escritura = string_from_format("PASOS=%s\nTIEMPO_PASOS=%s\n", consulta[REC_PASOS], consulta[REC_TIEMPOS]);

	char* infoRec = escribirNuevoArchivo(escritura, "Receta %s", consulta[REC_NOMBRE]);
	if(infoRec != NULL) {
		FILE* fd = fopen(path, "wt");
		fwrite(infoRec, strlen(infoRec), 1, fd);
		fclose(fd);
		CS_LOG_INFO("Se creo el archivo \"%s.AFIP\"", consulta[REC_NOMBRE]);

		free(infoRec);
	}

	free(path);
	free(escritura);
}

// --------------------- MANEJO BITMAP --------------------- //

int obtenerYEscribirProximoDisponible(char* aQuien){
	char* path = obtenerPathAbsoluto("Metadata/Bitmap.bin");

	sem_wait(&bitmapSem);
	int bitmapFile = open(path, O_CREAT | O_RDWR, 0664);

	void* punteroABitmap = mmap(NULL, cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, bitmapFile, 0);

	t_bitarray* bitmap = bitarray_create_with_mode((char*)punteroABitmap, cantidadBloques/8, MSB_FIRST);

	for(int i=0; i<cantidadBloques; i++){
		if(bitarray_test_bit(bitmap, i) == 0){
			bitarray_set_bit(bitmap ,i);
			CS_LOG_INFO("Se asigno el bloque %d al %s", i+1, aQuien);
			msync(bitmap->bitarray,cantidadBloques/8 ,0);
			close(bitmapFile);
			bitarray_destroy(bitmap);
			sem_post(&bitmapSem);
			free(path);
			return i + 1;
		}
	}
	close(bitmapFile);
	bitarray_destroy(bitmap);
	sem_post(&bitmapSem);
	free(path);

	CS_LOG_ERROR("BITMAP LLENO");
	return 0;
}

void eliminarBit(int index){
	char* path = obtenerPathAbsoluto("Metadata/Bitmap.bin");

	sem_wait(&bitmapSem);
	int bitmapFile = open(path, O_CREAT | O_RDWR, 0664);

	void* punteroABitmap = mmap(NULL, cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, bitmapFile, 0);

	t_bitarray* bitmap = bitarray_create_with_mode((char*)punteroABitmap, cantidadBloques/8, MSB_FIRST);

	bitarray_clean_bit(bitmap, index);
	CS_LOG_INFO("Se desasigno el bloque %d", index );

	msync(punteroABitmap ,cantidadBloques/8 ,0);

	close(bitmapFile);
	sem_post(&bitmapSem);
	bitarray_destroy(bitmap);
	free(path);
}

// --------------------- RECETA --------------------- //

int existeReceta(char* comida){
	char* path = obtenerPathAbsoluto("Files/Recetas/%s.AFIP", comida);
	int existe = existeDirectorio(path, 0);
	free(path);
	return existe;
}

t_rta_obt_rec* cs_lectura_to_receta(char* lectura) {
	t_dictionary* temp = cs_lectura_to_dictionary(lectura);

	t_rta_obt_rec* receta = cs_rta_obtener_receta_create(
			  dictionary_get(temp, "PASOS")
			, dictionary_get(temp, "TIEMPO_PASOS")
	);

	dictionary_destroy_and_destroy_elements(temp, (void*) free);

	return receta;
}

// --------------------- PEDIDO --------------------- //

int buscarPlatoEnPedido(char* escrituraVieja, t_consulta* consulta){
	t_rta_obt_ped* pedido = cs_lectura_to_pedido(escrituraVieja);

	bool encontrarPorNombre(t_plato* plato) {
		return !strcmp(plato->comida, consulta->comida);
	}
	t_plato * plato = list_find(pedido->platos_y_estados, (void*) encontrarPorNombre);
	return plato!=NULL;
}

t_rta_obt_ped* cs_lectura_to_pedido(char* lectura) {
	t_dictionary* temp = cs_lectura_to_dictionary(lectura);

	CS_LOG_TRACE("ESTADO_PEDIDO -> %s", dictionary_get(temp, "ESTADO_PEDIDO"));
	CS_LOG_TRACE("LISTA_PLATOS -> %s", dictionary_get(temp, "LISTA_PLATOS"));
	CS_LOG_TRACE("CANTIDAD_LISTA -> %s", dictionary_get(temp, "CANTIDAD_LISTA"));
	CS_LOG_TRACE("CANTIDAD_PLATOS -> %s", dictionary_get(temp, "CANTIDAD_PLATOS"));

	t_rta_obt_ped* pedido = cs_rta_obtener_ped_create(
			  cs_string_to_enum(dictionary_get(temp, "ESTADO_PEDIDO"), cs_enum_estado_pedido_to_str)
			, dictionary_get(temp, "LISTA_PLATOS")
			, dictionary_get(temp, "CANTIDAD_LISTA")
			, dictionary_get(temp, "CANTIDAD_PLATOS")
	);
	dictionary_destroy_and_destroy_elements(temp, (void*) free);

	return pedido;
}

int existePedido(int idPedido, char* nombreRestaurante){
	char* path = obtenerPathPedido(idPedido, nombreRestaurante);
	int resultado = false;
	if(fopen(path,"r") == NULL){
		resultado = false;
	} else {
		resultado = true;
	}
	free(path);
	return resultado;
}

void escribirInfoPedido(char* infoPedido, int idPedido, char* nombreRestaurante){
	char* path = obtenerPathPedido(idPedido, nombreRestaurante);
	FILE* file = fopen(path, "w");
	fwrite(infoPedido, 1, strlen(infoPedido), file);
	fclose(file);
	CS_LOG_INFO("Escribi el Info del pedido %d para el restaurante %s", idPedido, nombreRestaurante);
	free(path);
	free(infoPedido);
}

void obtenerMetadataPedido(int idPedido, char* restaurante, int* bloque, int* size){
	char* path = obtenerPathPedido(idPedido, restaurante);
	t_config* conf = config_create(path);
	*bloque = config_get_int_value(conf, "INITIAL_BLOCK");
	*size = config_get_int_value(conf, "SIZE");
	config_destroy(conf);
	free(path);
}

int estaEnEstado(char* lectura, e_estado_ped estado){
	char* renglon = string_from_format("ESTADO_PEDIDO=%s", cs_enum_estado_pedido_to_str(estado));
	int resultado = string_contains(lectura, renglon);
	free(renglon);
	return resultado;
}

char* obtenerPathPedido(int idPedido, char* nombreRestaurante) {
	return obtenerPathAbsoluto("Files/Restaurantes/%s/Pedido%d.AFIP", nombreRestaurante,idPedido);
}

char* leerPedido(uint32_t idPedido, char* nombreRestaurante) {
	char* lectura = NULL;
	if(existeRestaurante(nombreRestaurante)) {
		if(existePedido(idPedido, nombreRestaurante)) {
			int bloqueInicial, size;
			obtenerMetadataPedido(idPedido, nombreRestaurante, &bloqueInicial, &size);
			lectura = leerBloques(bloqueInicial, size);
		} else {
			CS_LOG_ERROR("No existe el Pedido %d", idPedido);
		}
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", nombreRestaurante);
	}

	return lectura;
}

// --------------------- MODIFICAR STRINGS --------------------- //

char* cs_pedido_to_escritura(t_rta_obt_ped* pedido) {
	char *comidas, *listos, *totales;
	cs_platos_to_string(pedido->platos_y_estados, &comidas, &listos, &totales);
	char* escritura = string_from_format(
			"ESTADO_PEDIDO=%s\n"
			"LISTA_PLATOS=%s\n"
			"CANTIDAD_PLATOS=%s\n"
			"CANTIDAD_LISTA=%s\n"
			"PRECIO_TOTAL=0\n"
			, cs_enum_estado_pedido_to_str(pedido->estado_pedido)
			, comidas
			, totales
			, listos
	);
	free(comidas);
	free(listos);
	free(totales);

	return escritura;
}

char* agregarCantPlatos(char* escrituraVieja, t_consulta* consulta){
	t_rta_obt_ped* pedido = cs_lectura_to_pedido(escrituraVieja);

	bool encontrarPorNombre(t_plato* plato) {
		return !strcmp(plato->comida, consulta->comida);
	}
	t_plato * plato = list_find(pedido->platos_y_estados, (void*) encontrarPorNombre);
	plato->cant_total += consulta->cantidad;

	char* escrituraNueva = cs_pedido_to_escritura(pedido);

	cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
	return escrituraNueva;
}

char* agregarPlato(char* escrituraVieja, t_consulta* consulta){ // LISTO
	t_rta_obt_ped* pedido = cs_lectura_to_pedido(escrituraVieja);

	t_plato* nuevo = malloc(sizeof(t_plato));
	nuevo->comida = string_duplicate(consulta->comida);
	nuevo->cant_lista = 0;
	nuevo->cant_total = consulta->cantidad;
	list_add(pedido->platos_y_estados, nuevo);

	char* escrituraNueva = cs_pedido_to_escritura(pedido);

	cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
	return escrituraNueva;
}

char* agregarPlatoListo(char* escrituraVieja, t_consulta* consulta){ // LISTO
	t_rta_obt_ped* pedido = cs_lectura_to_pedido(escrituraVieja);

	bool encontrarPorNombre(t_plato* plato) {
		return !strcmp(plato->comida, consulta->comida);
	}
	t_plato * plato = list_find(pedido->platos_y_estados, (void*) encontrarPorNombre);
	plato->cant_lista++;
	if(plato->cant_lista > plato->cant_total) {
		cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
		return NULL;
	}

	char* escrituraNueva = cs_pedido_to_escritura(pedido);

	cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
	return escrituraNueva;
}

char* cambiarEstadoPedidoA(char* lectura, t_consulta* consulta, e_estado_ped estado){ // LISTO
	t_rta_obt_ped* pedido = cs_lectura_to_pedido(lectura);

	pedido->estado_pedido = estado;

	char* escrituraNueva = cs_pedido_to_escritura(pedido);

	cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
	return escrituraNueva;
}

// --------------------- RETOCAR BLOQUES --------------------- //

e_opcode pisarPedido(uint32_t idPedido, char* nombreRestaurante, char* nuevaEscritura) {
	int bloqueInicial, size;
	obtenerMetadataPedido(idPedido, nombreRestaurante, &bloqueInicial, &size);
	t_list* bloques = leerNumerosBloques(bloqueInicial, size);
	ajustarCantidadBloques(bloques, nuevaEscritura, nombreRestaurante);
	int primerBloque = (int)list_get(bloques, 0); //Si la lista está vacía, retorna 0
	if(primerBloque > 0) {
		escribirBloques(bloques, nuevaEscritura);
		char* infoPedido = string_from_format("SIZE=%d\nINITIAL_BLOCK=%d", strlen(nuevaEscritura), primerBloque);
		escribirInfoPedido(infoPedido, idPedido, nombreRestaurante);
	} else {
		list_destroy(bloques);
		return OPCODE_RESPUESTA_FAIL;
	}
	list_destroy(bloques);
	return OPCODE_RESPUESTA_OK;
}

void ajustarCantidadBloques(t_list* bloques, char* escrituraNueva, char* aQuien) {
	int cantidadEscrituras = calcularCantBloques(strlen(escrituraNueva));
	while(list_size(bloques) > cantidadEscrituras){
		eliminarBit((int)list_get(bloques, list_size(bloques)-1));
		limpiarBloque((int)list_get(bloques, list_size(bloques)-1));
		list_remove(bloques, list_size(bloques)-1);
	}
	t_list* listaDeBloquesAgregados = list_create();
	while(list_size(bloques) < cantidadEscrituras){
		int bloqueNuevo = obtenerYEscribirProximoDisponible(aQuien);
		if(bloqueNuevo == 0){
			while(!list_is_empty(listaDeBloquesAgregados)){
				int bloqueALiberar = (int)list_remove(listaDeBloquesAgregados, 0);
				eliminarBit(bloqueALiberar);
			}
			list_destroy(listaDeBloquesAgregados);
			list_clean(bloques);
			return;
		}
		list_add(listaDeBloquesAgregados, (void*)bloqueNuevo);
	}
	list_add_all(bloques, listaDeBloquesAgregados);
	list_destroy(listaDeBloquesAgregados);
}

void escribirBloques(t_list* bloques, char* escrituraNueva){ //Antes era pisar()
	int numBloque = (int)list_get(bloques, 0);

	for(int i = 0; i < list_size(bloques) - 1; i++) {
		uint32_t proxBloque = (uint32_t)list_get(bloques, i + 1);
		char* escrituraAux = string_substring(escrituraNueva, i * (tamanioBloque - tamanioReservado), tamanioBloque - tamanioReservado);
		escrituraAux = realloc(escrituraAux, tamanioBloque);
		memcpy(escrituraAux + tamanioBloque - tamanioReservado, &proxBloque, tamanioReservado);
		escribirBloque(escrituraAux, numBloque);
		free(escrituraAux);
		numBloque = (int)proxBloque;
	}

	char* escrituraAux = malloc(tamanioBloque);
	strncpy(escrituraAux, escrituraNueva + (list_size(bloques) - 1) * (tamanioBloque - tamanioReservado), tamanioBloque);
	escribirBloque(escrituraAux, numBloque);
	free(escrituraAux);
}

// --------------------- BLOQUES --------------------- //

int calcularCantBloques(int size) {
	return (size + tamanioBloque - tamanioReservado - 1) / (tamanioBloque - tamanioReservado);
}

int escribirBloquesNuevos(char* escritura, char* aQuien){ // Antes era escribirBloques
	int cantidadEscrituras = calcularCantBloques(strlen(escritura));
	t_list* bloques = list_create();
	for(int i = 0; i < cantidadEscrituras; i++) {
		int bloqueNuevo = obtenerYEscribirProximoDisponible(aQuien);
		if(bloqueNuevo == 0) {
			while(!list_is_empty(bloques)){
				int bloqueALiberar = (int)list_remove(bloques, 0);
				eliminarBit(bloqueALiberar);
			}
			break;
		}
		list_add(bloques, (void*)bloqueNuevo);
	}
	int primerBloque = (int)list_get(bloques, 0);
	if(primerBloque > 0) {
		escribirBloques(bloques, escritura);
	}
	list_destroy(bloques);

	return primerBloque;
}

void escribirBloque(char* escritura, int numBloque){
	char* path = obtenerPathAbsoluto("Blocks/%d.AFIP", numBloque);
	FILE* fd = fopen(path, "wrb");
	fwrite(escritura, 1, tamanioBloque, fd);
	fclose(fd);
	CS_LOG_INFO("Escribi el bloque %d", numBloque);
	cs_log_hexdump(LOG_LEVEL_DEBUG, escritura, tamanioBloque);
	free(path);
}

char* leerBloque(int nroBloque) {
	char* path = obtenerPathAbsoluto("Blocks/%d.AFIP", nroBloque);
	char* lecturaAux = malloc(tamanioBloque);
	FILE* f = fopen(path, "r");

	fread(lecturaAux, 1, tamanioBloque, f);
	fclose(f);

	CS_LOG_DEBUG("Lei el bloque %d", nroBloque);
	cs_log_hexdump(LOG_LEVEL_DEBUG, lecturaAux, tamanioBloque);

	free(path);
	return lecturaAux;
}

char* leerBloques(int initialBlock, int size){
	char* lectura = string_new();
	int cantidadLecturas = calcularCantBloques(size);
	uint32_t nextBlock = initialBlock;

	for(int i = 1; i < cantidadLecturas; i++){
		char* lecturaAux = leerBloque(nextBlock);
		string_n_append(&lectura, lecturaAux, tamanioBloque - tamanioReservado);
		memcpy(&nextBlock, lecturaAux + tamanioBloque - tamanioReservado, tamanioReservado);
		free(lecturaAux);
	}
	char* lecturaAux = leerBloque(nextBlock);
	int resto = size % (tamanioBloque - tamanioReservado);
	if(resto == 0){
		resto = tamanioBloque;
	}
	string_n_append(&lectura, lecturaAux, resto);
	free(lecturaAux);
	return lectura;
}

t_list* leerNumerosBloques(int initialBlock, int size){
	int cantidadEscrituras = calcularCantBloques(size);
	t_list* bloques = list_create();
	list_add(bloques, (void*)initialBlock);
	uint32_t nextBlock = initialBlock;
	for(int i = 1; i < cantidadEscrituras; i++){
		char* lecturaAux = leerBloque(nextBlock);
		memcpy(&nextBlock, lecturaAux + tamanioBloque - tamanioReservado, tamanioReservado);
		list_add(bloques, (void*)nextBlock);
		free(lecturaAux);
	}
	return bloques;
}

void limpiarBloque(int bloque){
	char* pathBloque = obtenerPathAbsoluto("Blocks/%d.AFIP", bloque);
	FILE* block = fopen(pathBloque, "wrb");
	fseek(block, tamanioBloque - 1, SEEK_SET);
	fputc(0, block);
	fclose(block);
	free(pathBloque);
}

// --------------------- RESTAURANTE --------------------- //

bool buscarPlatoEnRestaurante(char* lectura, t_consulta* consulta){
	t_dictionary* temp = cs_lectura_to_dictionary(lectura);
	char** platos = string_get_string_as_array(dictionary_get(temp, "PLATOS"));
	int i = 0;
	bool resultado = false;
	while(platos[i] != NULL){
		if(!strcmp(platos[i], consulta->comida)){
			resultado = true;
			break;
		}
		i++;
	}
	liberar_lista(platos);
	dictionary_destroy_and_destroy_elements(temp, (void*) free);
	return resultado;
}

char* obtenerPlatos(char* lectura){
	t_dictionary* temp = cs_lectura_to_dictionary(lectura);
	char* retorno = dictionary_remove(temp, "PLATOS");
	dictionary_destroy_and_destroy_elements(temp, (void*) free);

	return retorno;
}

char* obtenerPathRestaurante(char* nombreRestaurante){
	return obtenerPathAbsoluto("Files/Restaurantes/%s", nombreRestaurante);
}

uint32_t obtenerCantidadPedidos(char* nombreRestaurante){
	uint32_t cant = 0;
	char* path = obtenerPathRestaurante(nombreRestaurante);
	char* comando = string_from_format("ls %s | wc -l", path);
	FILE *pipe = popen(comando, "r");
	if(pipe != NULL) {
		char* buffer = NULL;
		int len = 0;
		do {
			len++;
			buffer = realloc(buffer, len);
			buffer[len-1] = fgetc(pipe);
		} while(buffer[len-1] != '\n');
		buffer[len-1] = '\0';
		cant = atoi(buffer);
		pclose(pipe);
		free(buffer);
	}
	free(path);
	free(comando);

	return cant - 1;
}

t_rta_obt_rest* cs_lectura_to_restaurante(char* lectura, uint32_t cant_pedidos){
	t_dictionary* temp = cs_lectura_to_dictionary(lectura);
	char** pos_lista = string_get_string_as_array(dictionary_get(temp, "POSICION"));
	t_pos posicion = cs_string_array_to_pos(pos_lista);
	liberar_lista(pos_lista);

	t_rta_obt_rest* restaurante = cs_rta_obtener_rest_create(
			  atoi(dictionary_get(temp, "CANTIDAD_COCINEROS"))
			, dictionary_get(temp, "AFINIDAD_COCINEROS")
			, dictionary_get(temp, "PLATOS")
			, dictionary_get(temp, "PRECIO_PLATOS")
			, posicion
			, atoi(dictionary_get(temp, "CANTIDAD_HORNOS"))
			, cant_pedidos
	);

	dictionary_destroy_and_destroy_elements(temp, (void*) free);

	return restaurante;
}

bool existeRestaurante(char* restaurante) {
	char* path = obtenerPathRestaurante(restaurante);
	bool existe = existeDirectorio(path, 0);
	free(path);
	return existe;
}

char* leerRestaurante(char* restaurante) {
	char* lectura = NULL;
	if(existeRestaurante(restaurante)) {
		char* path = obtenerPathAbsoluto("Files/Restaurantes/%s/Info.AFIP", restaurante);
		t_config* md = config_create(path);
		if(md != NULL) {
			int initialBlock = config_get_int_value(md, "INITIAL_BLOCK");
			int size = config_get_int_value(md, "SIZE");
			lectura = leerBloques(initialBlock, size);
			config_destroy(md);
		} else {
			CS_LOG_ERROR("No existe %s/Info.AFIP", restaurante);
		}
		free(path);
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", restaurante);
	}

	return lectura;
}

// --------------------- AUX --------------------- //

int existeDirectorio(char* path, int creacion){
	if(creacion){
		return mkdir(path, 0777);
	} else {
		int existe = mkdir(path, 0777);
		if(!existe){
			rmdir(path);
		}
		return existe;
	}
}

int tamanioArchivo(char* path){
	FILE* fd = fopen(path, "rb");
	fseek(fd, 0, SEEK_END);
	int tamanio = ftell(fd);
	fclose(fd);
	return tamanio;
}

void liberar_lista(char** lista){
	string_iterate_lines(lista, (void*) free);
	free(lista);
}

char* obtenerPathAbsoluto(const char* pathRelativo, ...) {
	char* format = string_from_format("%s/%s", puntoMontaje, pathRelativo);

	va_list arguments;
	va_start(arguments, pathRelativo);
	char* pathAbsoluto = string_from_vformat(format, arguments);
	va_end(arguments);

	free(format);
	return pathAbsoluto;
}

t_dictionary* cs_lectura_to_dictionary(char* lectura) {
	t_dictionary* dictionary = dictionary_create();
	char** lineas = string_split(lectura, "\n");
	for(int i = 0; lineas[i] != NULL; i++) {
		char** aux = string_n_split(lineas[i], 2, "=");
		dictionary_put(dictionary, aux[0], aux[1]);
		free(aux[0]);
		free(aux);
	}
	liberar_lista(lineas);

	return dictionary;
}

char* escribirNuevoArchivo(char* escritura, const char* dest, ...) {
	va_list args;
	va_start(args, dest);
	char* aQuien = string_from_vformat(dest, args);
	va_end(args);
	int primerBloque = escribirBloquesNuevos(escritura, aQuien);
	free(aQuien);
	if(primerBloque > 0){
		return string_from_format("SIZE=%d\nINITIAL_BLOCK=%d", strlen(escritura), primerBloque);
	} else {
		return NULL;
	}
}
