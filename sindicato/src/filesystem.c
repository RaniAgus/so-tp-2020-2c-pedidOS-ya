#include "filesystem.h"

// --------------------- LEVANTAR FILE SYSTEM --------------------- //

void crearDirectorioAFIP(){
	obtenerMetadata();
	char* pathBitmap = obtenerPathAbsoluto("Metadata/Bitmap.bin");

	if(fopen(pathBitmap, "r") == NULL){
		//CS_LOG_INFO("Genere el directorio %s", puntoMontaje);
		generarBitmap();
		crearFiles();
		crearBlocks();
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
		char* pathBloque = obtenerPathAbsoluto("%s/%d.AFIP", pathBlocks, i);
		FILE* block = fopen(pathBloque, "wrb");
		fclose(block);
		free(pathBloque);
	}
	CS_LOG_INFO("Se agregaron %d archivos BLOCKS", cantidadBloques);

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
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){
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
			char* megaaux = string_from_format("Pedido %d",consulta->pedido_id);
			int primerBloque = escribirBloques(escritura, megaaux);
			free(megaaux);
			if(primerBloque){
				char* infoPedido = string_from_format("SIZE=%d\nINITIAL_BLOCK=%d", strlen(escritura), primerBloque);
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
	free(path);
	return respuesta;
}

e_opcode guardarPlato(t_consulta* consulta){ // LISTO
	e_opcode respuesta = OPCODE_RESPUESTA_FAIL;
	char* lectura = leerPedido(consulta->pedido_id, consulta->restaurante);
	if(lectura != NULL) {
		if(estaEnEstado(lectura, PEDIDO_PENDIENTE)){
			char* nuevaEscritura;
			if(string_contains(lectura, consulta->comida)){
				nuevaEscritura = agregarCantPlatos(lectura ,consulta);
			} else {
				nuevaEscritura = agregarPlato(lectura, consulta);
			}
			pisarPedido(consulta->pedido_id, consulta->restaurante, nuevaEscritura);
			free(nuevaEscritura);
			respuesta = OPCODE_RESPUESTA_OK;
		} else {
			CS_LOG_ERROR("El pedido %d ya no se encuentra pendiente", consulta->pedido_id);
		}
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
			pisarPedido(consulta->pedido_id, consulta->restaurante, nuevaEscritura);
			free(nuevaEscritura);
			respuesta = OPCODE_RESPUESTA_OK;
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
			if(string_contains(lectura, consulta->comida)){
				char* nuevaEscritura = agregarPlatoListo(lectura, consulta);
				if(nuevaEscritura != lectura) {
					pisarPedido(consulta->pedido_id, consulta->restaurante, nuevaEscritura);
					free(nuevaEscritura);
					respuesta = OPCODE_RESPUESTA_OK;
				} else {
					CS_LOG_ERROR("Ya estaban listos todos los platos %s del pedido %s del restaurante %s", consulta->comida, consulta->pedido_id, consulta->restaurante);
				}
			} else {
				CS_LOG_ERROR("El pedido %d del restaurante %s no posee el plato %s", consulta->pedido_id, consulta->restaurante, consulta->comida);
			}
		} else {
			CS_LOG_ERROR("El pedido %d todavía no había sido confirmado", consulta->pedido_id);
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
			pisarPedido(consulta->pedido_id, consulta->restaurante, nuevaEscritura);
			free(nuevaEscritura);
			respuesta = OPCODE_RESPUESTA_OK;
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
	CS_LOG_INFO("Se creo el restaurante %s", consulta[RES_NOMBRE]);

	string_append(&path, "/info.AFIP");
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

	char* aux = string_from_format("Restaurante %s",consulta[RES_NOMBRE]);
	char* infoRes = string_from_format(
			"SIZE=%d\n"
			"INITIAL_BLOCK=%d\n"
			, strlen(escritura)
			, escribirBloques(escritura, aux)
	);
	free(aux);

	FILE* fd = fopen(path, "wt");
	fwrite(infoRes, strlen(infoRes), 1, fd);
	fclose(fd);
	CS_LOG_INFO("Se creo el archivo \"info.AFIP\" para el restaurant %s", consulta[RES_NOMBRE]);

	free(infoRes);
	free(escritura);
	free(path);
}

void crearReceta(char** consulta){ // LISTO
	char* path = obtenerPathAbsoluto("Files/Recetas/%s.AFIP", consulta[REC_NOMBRE]);
	char* escritura = string_from_format("PASOS=%s\nTIEMPO_PASOS=%s\n", consulta[REC_PASOS], consulta[REC_TIEMPOS]);

	char* aux = string_from_format("Receta %s",consulta[REC_NOMBRE]);
	char* infoRes = string_from_format(
		"SIZE=%d\n"
		"INITIAL_BLOCK=%d\n"
		, strlen(escritura)
		, escribirBloques(escritura, aux)
	);
	free(aux);

	FILE* fd = fopen(path, "wt");
	fwrite(infoRes, strlen(infoRes), 1, fd);
	fclose(fd);
	CS_LOG_INFO("Se creo el archivo \"%s.AFIP\"", consulta[REC_NOMBRE]);

	free(infoRes);
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

	for(int i=1; i<=cantidadBloques; i++){
		if(bitarray_test_bit(bitmap, i) == 0){
			bitarray_set_bit(bitmap ,i);
			CS_LOG_INFO("Se asigno el bloque %d al %s", i, aQuien);
			msync(bitmap->bitarray,cantidadBloques/8 ,0);
			close(bitmapFile);
			bitarray_destroy(bitmap);
			sem_post(&bitmapSem);
			free(path);
			return i;
		}
	}
	close(bitmapFile);
	bitarray_destroy(bitmap);
	sem_post(&bitmapSem);
	free(path);

	CS_LOG_TRACE("BITMAP LLENO");
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
	int file = open(path, O_CREAT | O_RDWR, 0664);
	lseek(file, 0, SEEK_END);
	write(file, infoPedido, strlen(infoPedido));
	close(file);
	CS_LOG_INFO("Escribi el Info del pedido %d para el restaurante %s", idPedido, nombreRestaurante);
	free(path);
	free(infoPedido);
}

int obtenerBlockInicialPedido(int idPedido, char* restaurante){
	char* path = obtenerPathPedido(idPedido, restaurante);
	t_config* conf = config_create(path);
	int bloque = config_get_int_value(conf, "INITIAL_BLOCK");
	config_destroy(conf);
	free(path);
	return bloque;
}

int obtenerTamanioPedido(int idPedido, char* restaurante){
	char* path = obtenerPathPedido(idPedido, restaurante);
	t_config* conf = config_create(path);
	int size = config_get_int_value(conf, "SIZE");
	config_destroy(conf);
	free(path);
	return size;
}

int estaEnEstado(char* lectura, e_estado_ped estado){
	char* renglon = string_from_format("ESTADO_PEDIDO=%s", cs_enum_estado_pedido_to_str(estado));
	int resultado = string_contains(lectura, renglon);
	free(renglon);
	return resultado;
}

t_rta_obt_ped* cs_lectura_to_pedido(char* lectura) {
	t_dictionary* temp = cs_lectura_to_dictionary(lectura);

	t_rta_obt_ped* pedido = cs_rta_obtener_ped_create(
			  cs_string_to_enum(dictionary_get(temp, "ESTADO_PEDIDO"), cs_enum_estado_pedido_to_str)
			, dictionary_get(temp, "LISTA_PLATOS")
			, dictionary_get(temp, "CANTIDAD_LISTA")
			, dictionary_get(temp, "CANTIDAD_PLATOS")
	);
	dictionary_destroy_and_destroy_elements(temp, (void*) free);

	return pedido;
}

char* obtenerPathPedido(int idPedido, char* nombreRestaurante) {
	return obtenerPathAbsoluto("Files/Restaurantes/%s/Pedido%d", nombreRestaurante,idPedido);
}

char* leerPedido(uint32_t idPedido, char* nombreRestaurante) {
	char* lectura = NULL;
	if(existeRestaurante(nombreRestaurante)) {
		if(existePedido(idPedido, nombreRestaurante)) {
			int bloqueInicial = obtenerBlockInicialPedido(idPedido, nombreRestaurante);
			int size = obtenerTamanioPedido(idPedido, nombreRestaurante);
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

char* agregarCantPlatos(char* escrituraVieja, t_consulta* consulta){
	t_rta_obt_ped* pedido = cs_lectura_to_pedido(escrituraVieja);

	bool encontrarPorNombre(t_plato* plato) {
		return !strcmp(plato->comida, consulta->comida);
	}
	t_plato * plato = list_find(pedido->platos_y_estados, (void*) encontrarPorNombre);
	plato->cant_total += consulta->cantidad;

	char *comidas, *listos, *totales;
	cs_platos_to_string(pedido->platos_y_estados, &comidas, &listos, &totales);
	char* respuesta = string_from_format(
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

	cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);
	return respuesta;
}

char* agregarPlato(char* escrituraVieja, t_consulta* consulta){ // LISTO
	char* respuesta;
	char* leido;
	char* nuevo;
	char* aux;
	char* path = obtenerPathPedido(consulta->pedido_id, consulta->restaurante);
	FILE* f = fopen(path, "wrb");
	fwrite(escrituraVieja, 1, strlen(escrituraVieja), f);
	fclose(f);
	t_config* conf = config_create(path);
	leido = config_get_string_value(conf, "LISTA_PLATOS");
	if(strlen(leido)==2){
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s%s]", aux, consulta->comida);
		free(aux);
	} else {
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s,%s]", aux, consulta->comida);
		free(aux);
	}
	config_set_value(conf, "LISTA_PLATOS", nuevo);
	free(nuevo);
	leido = config_get_string_value(conf, "CANTIDAD_LISTA");
	if(strlen(leido)==2){
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s%d]", aux, 0);
		free(aux);
	} else {
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s,%d]",aux, 0);
		free(aux);
	}
	config_set_value(conf, "CANTIDAD_LISTA", nuevo);
	free(nuevo);
	leido = config_get_string_value(conf, "CANTIDAD_PLATOS");
	if(strlen(leido)==2){
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s%d]", aux, consulta->cantidad);
		free(aux);
	} else {
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s,%d]", aux, consulta->cantidad);
		free(aux);
	}
	config_set_value(conf, "CANTIDAD_PLATOS", nuevo);
	config_save(conf);
	config_destroy(conf);
	free(nuevo);
	int tamanio = tamanioArchivo(path);
	f = fopen(path, "r");
	respuesta = malloc(tamanio);
	fread(respuesta, 1, tamanio, f);
	fclose(f);
	remove(path);
	free(path);
	return respuesta;
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
		return escrituraVieja;
	}

	char *comidas, *listos, *totales;
	cs_platos_to_string(pedido->platos_y_estados, &comidas, &listos, &totales);
	char* respuesta = string_from_format(
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
	cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);

	return respuesta;
}

char* cambiarEstadoPedidoA(char* lectura, t_consulta* consulta, e_estado_ped estado){ // LISTO
	t_rta_obt_ped* pedido = cs_lectura_to_pedido(lectura);
	pedido->estado_pedido = estado;

	char *comidas, *listos, *totales;
	cs_platos_to_string(pedido->platos_y_estados, &comidas, &listos, &totales);
	char* respuesta = string_from_format(
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
	cs_msg_destroy(pedido, OPCODE_RESPUESTA_OK, OBTENER_PEDIDO);

	return respuesta;
}

// --------------------- RETOCAR BLOQUES --------------------- //

void pisarPedido(uint32_t idPedido, char* nombreRestaurante, char* nuevaEscritura) {
	int bloqueInicial = obtenerBlockInicialPedido(idPedido, nombreRestaurante);
	int size = obtenerTamanioPedido(idPedido, nombreRestaurante);
	t_list* bloques = leerNumerosBloques(bloqueInicial, size);
	pisar(bloques, nuevaEscritura, nombreRestaurante);
	list_destroy(bloques);
}

void pisar(t_list* bloques, char* escrituraNueva, char* restaurante){ // REVISAR REALLOC
	int cantidadEscrituras = (strlen(escrituraNueva) + tamanioBloque - tamanioReservado -1)/(tamanioBloque-tamanioReservado);
	int bloqueNuevo;
	while(list_size(bloques) > cantidadEscrituras){
		eliminarBit((int)list_get(bloques, list_size(bloques)-1));
		limpiarBloque((int)list_get(bloques, list_size(bloques)-1));
		list_remove(bloques, list_size(bloques)-1);
	}
	while(list_size(bloques) < cantidadEscrituras){
		bloqueNuevo = obtenerYEscribirProximoDisponible(restaurante);
		list_add(bloques, (void*)bloqueNuevo);
	}
	int puntero=0;
	uint32_t nextBlock;
	char* unaux;
	char* escrituraAux;
	for(int i=0; i<list_size(bloques)-1; i++){
		unaux = string_substring(escrituraNueva ,puntero, tamanioBloque-tamanioReservado);
		escrituraAux = string_from_format("%s", unaux);
		escrituraAux = realloc(escrituraAux, tamanioBloque);
		nextBlock = (uint32_t)list_get(bloques, i+1);
		memcpy(escrituraAux + tamanioBloque - tamanioReservado, &nextBlock, tamanioReservado);
		escribirBloque(escrituraAux, (int)list_get(bloques,i));
		puntero = puntero + strlen(unaux);
		free(unaux);
		free(escrituraAux);
	}
	unaux = string_substring_from(escrituraNueva, puntero);
	escrituraAux = string_from_format("%s", unaux);
	escribirBloque(escrituraAux, (int)list_get(bloques, list_size(bloques)-1));
	free(unaux);
	free(escrituraAux);
}

// --------------------- BLOQUES --------------------- //

int escribirBloques(char* escritura, char* aQuien){
	int primerBloque = 0;
	int puntero = 0;
	int cantidadEscrituras =  (strlen(escritura) + tamanioBloque - tamanioReservado -1)/(tamanioBloque-tamanioReservado);
	int numBloque = obtenerYEscribirProximoDisponible(aQuien);
	uint32_t proxBloque = obtenerYEscribirProximoDisponible(aQuien);
	primerBloque = numBloque;
	char* escrituraAux = string_substring(escritura, puntero, tamanioBloque - tamanioReservado);
	puntero = puntero + strlen(escrituraAux);
	escrituraAux = realloc(escrituraAux, tamanioBloque);
	memcpy(escrituraAux + tamanioBloque - tamanioReservado, &proxBloque, tamanioReservado);
	escribirBloque(escrituraAux, numBloque);
	free(escrituraAux);
	for(int i=0; i<cantidadEscrituras-2; i++){
		numBloque = (int)proxBloque;
		proxBloque = (uint32_t)obtenerYEscribirProximoDisponible(aQuien);
		escrituraAux = string_substring(escritura, puntero, tamanioBloque - tamanioReservado);
		puntero = puntero + strlen(escrituraAux);
		escrituraAux = realloc(escrituraAux, tamanioBloque);
		memcpy(escrituraAux + tamanioBloque - tamanioReservado, &proxBloque, tamanioReservado);
		escribirBloque(escrituraAux, numBloque);
		free(escrituraAux);
	}
	escrituraAux = string_substring_from(escritura, puntero);
	escribirBloque(escrituraAux, proxBloque);
	free(escrituraAux);
	return primerBloque;
}

void escribirBloque(char* escritura, int numBloque){
	char* path = string_from_format("%s/Blocks/%d.AFIP", puntoMontaje, numBloque);
	FILE* fd = fopen(path, "wrb");
	fwrite(escritura, 1, tamanioBloque, fd);
	fclose(fd);
	CS_LOG_INFO("Escribi el bloque %d", numBloque);
	free(path);
}

char* leerBloques(int initialBlock, int size){
	char* lectura = string_new();
	char* lecturaAux = malloc(tamanioBloque);
	int cantidadLecturas =  (size + tamanioBloque - tamanioReservado -1)/(tamanioBloque-tamanioReservado);
	uint32_t nextBlock = 0;
	char* path = string_from_format("%s/Blocks/%d.AFIP", puntoMontaje, initialBlock);
	FILE* f = fopen(path, "r");
	fseek(f, 0L, SEEK_END);
	int tamanioArchivo = ftell(f);
	fseek(f, 0L, SEEK_SET);
	char* until;
	int i = 1;
	while(i < cantidadLecturas){
		fread(lecturaAux, 1, tamanioBloque, f);
		until = string_substring_until(lecturaAux, tamanioBloque - tamanioReservado);
		string_append(&lectura, until);
		free(until);
		memcpy(&nextBlock, lecturaAux + tamanioBloque - tamanioReservado, tamanioReservado);
		fclose(f);
		free(path);
		path = string_from_format("%s/Blocks/%d.AFIP", puntoMontaje, (int)nextBlock);
		f = fopen(path, "r");
		fseek(f, 0L, SEEK_END);
		tamanioArchivo = ftell(f);
		fseek(f, 0L, SEEK_SET);
		i++;
	}
	fread(lecturaAux, 1, tamanioArchivo, f);
	until = string_substring_until(lecturaAux, size%tamanioBloque);
	string_append(&lectura, until);
	free(until);
	fclose(f);
	free(path);
	free(lecturaAux);
	return lectura;
}

t_list* leerNumerosBloques(int initialBlock, int size){
	int cantidadEscrituras =  (size + tamanioBloque - tamanioReservado -1)/(tamanioBloque-tamanioReservado);
	t_list* bloques = list_create();
	list_add(bloques, (void*)initialBlock);
	uint32_t block;
	char* lecturaAux = malloc(tamanioBloque);
	char* path = string_from_format("%s/Blocks/%d.AFIP", puntoMontaje, initialBlock);
	FILE* f = fopen(path, "r");
	fseek(f, 0L, SEEK_END);
	fseek(f, 0L, SEEK_SET);
	int i = 1;
	while(i < cantidadEscrituras){
		fread(lecturaAux, tamanioBloque, 1, f);
		memcpy(&block, lecturaAux + tamanioBloque - tamanioReservado, tamanioReservado);
		list_add(bloques, (void*)block);
		fclose(f);
		free(path);
		path = string_from_format("%s/Blocks/%d.AFIP", puntoMontaje, (int)block);
		f = fopen(path, "r");
		fseek(f, 0L, SEEK_END);
		fseek(f, 0L, SEEK_SET);
		i++;
	}
	fclose(f);
	free(path);
	free(lecturaAux);
	return bloques;
}

void limpiarBloque(int bloque){
	char* path = obtenerPathAbsoluto("Blocks/%d.AFIP", bloque);
	FILE* f = fopen(path, "wrb");
	fclose(f);
}

// --------------------- RESTAURANTE --------------------- //

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
	char* bash_cmd = string_from_format("ls %s | wc -l", path);
	char* buffer = malloc(1000);
	FILE *pipe;
	int len;
	pipe = popen(bash_cmd, "r");
	if (NULL != pipe) {
		fgets(buffer, 1000, pipe);
		len = strlen(buffer);
		buffer[len-1] = '\0';
		cant = atoi(buffer);
		pclose(pipe);
		free(bash_cmd);
		free(buffer);
		free(path);
	}

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
		char* path = obtenerPathAbsoluto("Files/Restaurantes/%s/info.AFIP", restaurante);
		t_config* md = config_create(path);
		if(md != NULL) {
			int initialBlock = config_get_int_value(md, "INITIAL_BLOCK");
			int size = config_get_int_value(md, "SIZE");
			lectura = leerBloques(initialBlock, size);
			config_destroy(md);
		} else {
			CS_LOG_ERROR("No existe %s/info.AFIP", restaurante);
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
