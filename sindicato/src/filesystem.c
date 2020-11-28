#include "filesystem.h"

// --------------------- LEVANTAR FILE SYSTEM --------------------- //

void crearDirectorioAFIP(){
	char* path = string_new();
	string_append(&path,puntoMontaje);

	char* metadata = string_duplicate(path);
	string_append(&metadata, "/Metadata/Metadata.AFIP");

	char* bitmap = string_duplicate(path);
	string_append(&bitmap, "/Metadata/Bitmap.bin");
	crearMetadata(path);

	if(fopen(metadata,"r") == NULL || fopen(bitmap, "r") == NULL){
		mkdir(puntoMontaje, 0777);
		CS_LOG_INFO("Genere el directorio %s", puntoMontaje);
		crearMetadata(path);
		generarBitmap(path);
		crearFiles(path);
		crearBlocks(path);
	} else {
		free(path);
		CS_LOG_TRACE("Ya existe el FL\n");
	}

	free(metadata);
	free(bitmap);
}


void crearMetadata(char* pathOrigin){
	char* path = string_duplicate(pathOrigin);

	string_append(&path,"/Metadata");
	char* pathMetadata = string_duplicate(path);
	//if(existeDirectorio(path, 1)){
	//CS_LOG_TRACE("Ya existe el Metadata");
	string_append(&path, "/Metadata.AFIP");
	t_config* md = config_create(path);
	tamanioBloque = config_get_int_value(md, "BLOCK_SIZE");
	cantidadBloques = config_get_int_value(md, "BLOCKS");
	config_destroy(md);
	//} else {
	//	CS_LOG_ERROR("No existe Metadata");
	//	exit(-1);
	//}
	free(pathMetadata);
	free(path);
}


void crearFiles(char* pathOrigin){
	char* path = string_duplicate(pathOrigin);

	string_append(&path, "/Files");
	mkdir(path, 0777);
	string_append(&path, "/Restaurantes");
	mkdir(path, 0777);
	free(path);
	path = string_duplicate(pathOrigin);
	string_append(&path, "/Files/Recetas");
	mkdir(path, 0777);

	CS_LOG_INFO("Agrego directorio Files");
	CS_LOG_INFO("Agrego directorio Restaurantes");
	CS_LOG_INFO("Agrego directorio Recetas");
	free(path);
}


void crearBlocks(char* path){
	string_append(&path, "/Blocks");
	mkdir(path, 0777);
	CS_LOG_INFO("Agrego el directorio Blocks");
	FILE* block;
	char* pathAux;
	for(int i=1; i<=cantidadBloques; i++){
		pathAux = string_from_format("%s/%d.AFIP", path, i);
		block = fopen(pathAux, "wrb");
		fclose(block);
		free(pathAux);
	}
	CS_LOG_INFO("Se agregaron %d archivos BLOCKS", cantidadBloques);
	free(path);
}


void generarBitmap(char* pathOrigin){

	char* path = string_duplicate(pathOrigin);
	string_append(&path,"/Metadata");
	string_append(&path, "/Bitmap.bin");

	void* punteroABitmap = malloc(cantidadBloques/8);

	t_bitarray* bitmap = bitarray_create_with_mode(punteroABitmap, cantidadBloques/8, MSB_FIRST);
	for(int i = 0; i < cantidadBloques; i++){
		bitarray_clean_bit(bitmap, i);
	}
	FILE* bitmapFile = fopen(path, "wrb");
	fwrite(punteroABitmap, cantidadBloques/8, 1, bitmapFile);
	fclose(bitmapFile);
	bitarray_destroy(bitmap);
	free(punteroABitmap);
	free(path);

	CS_LOG_INFO("Genere el bitmap");
}


// --------------------------------------------------------- //
// ----------------- MENSAJES COMPONENTES ------------------ //
// --------------------------------------------------------- //

t_rta_cons_pl* consultarPlatos(t_consulta* consulta){ // LISTO
	char* path = obtenerPathRestaurante(consulta->restaurante);
	t_rta_cons_pl* respuesta;
	if(existeDirectorio(path, 0)){
		string_append(&path, "/info.AFIP");
		t_config* md = config_create(path);
		int initialBlock = config_get_int_value(md, "INITIAL_BLOCK");
		int size = config_get_int_value(md, "SIZE");
		config_destroy(md);
		char* lectura = leerBloques(initialBlock, size);
		char* platos = obtenerPlatos(lectura);
		respuesta = cs_rta_consultar_pl_create(platos);
		free(lectura);
		free(platos);
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
		respuesta = NULL;
	}
	free(path);
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
	e_opcode respuesta;
	respuesta = OPCODE_RESPUESTA_FAIL;
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){
		if(existePedido(consulta->pedido_id, consulta->restaurante)){
			int bloqueInicial = obtenerBlockInicialPedido(consulta->pedido_id, consulta->restaurante);
			int size = obtenerTamanioPedido(consulta->pedido_id, consulta->restaurante);
			char* lectura = leerBloques(bloqueInicial, size);
				if(estaEnEstado(lectura, PEDIDO_PENDIENTE)){
					t_list* bloques = leerNumerosBloques(bloqueInicial, size);
					char* nuevaEscritura;
					if(string_contains(lectura, consulta->comida)){
						nuevaEscritura = agregarCantPlatos(lectura ,consulta);
					} else {
						nuevaEscritura = agregarPlato(lectura, consulta);
					}
					pisar(bloques, nuevaEscritura, consulta->restaurante);
					respuesta = OPCODE_RESPUESTA_OK;
					free(nuevaEscritura);
					list_clean(bloques);
					list_destroy(bloques);
				} else {
					CS_LOG_ERROR("El pedido %d ya no se encuentra pendiente", consulta->pedido_id);
				}
			free(lectura);
		} else {
			CS_LOG_ERROR("No existe el Pedido %d", consulta->pedido_id);
		}
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

e_opcode confirmarPedido(t_consulta* consulta){ // LISTO
	e_opcode respuesta;
	respuesta = OPCODE_RESPUESTA_FAIL;
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){
		if(existePedido(consulta->pedido_id, consulta->restaurante)){
			int bloqueInicial = obtenerBlockInicialPedido(consulta->pedido_id, consulta->restaurante);
			int size = obtenerTamanioPedido(consulta->pedido_id, consulta->restaurante);
			char* lectura = leerBloques(bloqueInicial, size);
				if(estaEnEstado(lectura, PEDIDO_PENDIENTE)){
					t_list* bloques = leerNumerosBloques(bloqueInicial, size);
					char* nuevaEscritura = cambiarEstadoPedidoA(lectura, consulta, PEDIDO_CONFIRMADO);
					pisar(bloques, nuevaEscritura, consulta->restaurante);
					respuesta = OPCODE_RESPUESTA_OK;
					free(nuevaEscritura);
					free(bloques);
				} else {
					CS_LOG_ERROR("El pedido %d ya no se encuentra pendiente", consulta->pedido_id);
				}
			free(lectura);
		} else {
			CS_LOG_ERROR("No existe el Pedido %d", consulta->pedido_id);
		}
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

t_rta_obt_ped* obtenerPedido(t_consulta* consulta){ // LISTO
	t_rta_obt_ped* respuesta;
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){
		if(existePedido(consulta->pedido_id, consulta->restaurante)){
			int bloqueInicial = obtenerBlockInicialPedido(consulta->pedido_id, consulta->restaurante);
			int size = obtenerTamanioPedido(consulta->pedido_id, consulta->restaurante);
			char* lectura = leerBloques(bloqueInicial, size);
			respuesta = leerPedido(lectura);
			free(lectura);
		} else {
			CS_LOG_ERROR("No existe el Pedido %d", consulta->pedido_id);
		}
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

t_rta_obt_rest* obtenerRestaurante(t_consulta* consulta){  // LISTO
	t_rta_obt_rest* respuesta = NULL;
	char* path = obtenerPathRestaurante(consulta->restaurante);
	uint32_t cantPedidos = obtenerCantidadPedidos(consulta->restaurante);
	if(existeDirectorio(path, 0)){
		string_append(&path, "/info.AFIP");
		t_config* conf = config_create(path);
		int bloque = config_get_int_value(conf, "INITIAL_BLOCK");
		int size = config_get_int_value(conf, "SIZE");
		config_destroy(conf);
		char* lectura = leerBloques(bloque, size);
		respuesta = cs_string_to_restaurante(lectura, cantPedidos);
		free(lectura);
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

e_opcode platoListo(t_consulta* consulta){ //
	e_opcode respuesta;
	respuesta = OPCODE_RESPUESTA_FAIL;
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){
		if(existePedido(consulta->pedido_id, consulta->restaurante)){
					int bloqueInicial = obtenerBlockInicialPedido(consulta->pedido_id, consulta->restaurante);
					int size = obtenerTamanioPedido(consulta->pedido_id, consulta->restaurante);
					char* lectura = leerBloques(bloqueInicial, size);
						if(estaEnEstado(lectura, PEDIDO_CONFIRMADO)){
							if(string_contains(lectura, consulta->comida)){
								t_list* bloques = leerNumerosBloques(bloqueInicial, size);
								char* nuevaEscritura = agregarPlatoListo(lectura, consulta);
								pisar(bloques, nuevaEscritura, consulta->restaurante);
								respuesta = OPCODE_RESPUESTA_OK;
								free(nuevaEscritura);
								free(bloques);
							} else {
								CS_LOG_ERROR("El pedido %d del restaurante %s no posee el plato %s", consulta->pedido_id, consulta->restaurante, consulta->comida);
							}
						} else {
							CS_LOG_ERROR("El pedido %d todavía no había sido confirmado", consulta->pedido_id);
						}
					free(lectura);
				} else {
					CS_LOG_ERROR("No existe el Pedido %d", consulta->pedido_id);
				}
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

e_opcode terminarPedido(t_consulta* consulta){ //
	e_opcode respuesta;
		respuesta = OPCODE_RESPUESTA_FAIL;
		char* path = obtenerPathRestaurante(consulta->restaurante);
		if(existeDirectorio(path, 0)){
			if(existePedido(consulta->pedido_id, consulta->restaurante)){
				int bloqueInicial = obtenerBlockInicialPedido(consulta->pedido_id, consulta->restaurante);
				int size = obtenerTamanioPedido(consulta->pedido_id, consulta->restaurante);
				char* lectura = leerBloques(bloqueInicial, size);
					if(estaEnEstado(lectura, PEDIDO_CONFIRMADO)){
						t_list* bloques = leerNumerosBloques(bloqueInicial, size);
						char* nuevaEscritura = cambiarEstadoPedidoA(lectura, consulta, PEDIDO_TERMINADO);
						pisar(bloques, nuevaEscritura, consulta->restaurante);
						respuesta = OPCODE_RESPUESTA_OK;
						free(nuevaEscritura);
						free(bloques);
					} else {
						CS_LOG_ERROR("El pedido %d tovadía no se ha confirmado", consulta->pedido_id);
					}
				free(lectura);
			} else {
				CS_LOG_ERROR("No existe el Pedido %d", consulta->pedido_id);
			}
		} else {
			CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
		}
		free(path);
		return respuesta;

}

t_rta_obt_rec* obtenerReceta(t_consulta* consulta){ // LISTO
	t_rta_obt_rec* respuesta = NULL;
	char* path = string_from_format( "%s/Files/Recetas/%s.AFIP",
			puntoMontaje,
			consulta->comida
			);
	if(existeDirectorio(path, 0)){
		t_config* conf = config_create(path);
		int bloque = config_get_int_value(conf, "INITIAL_BLOCK");
		int size = config_get_int_value(conf, "SIZE");
		config_destroy(conf);
		char* lectura = leerBloques(bloque, size);
		respuesta = cs_string_to_receta(lectura);
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
	char* path = obtenerPathRestaurante(consulta[1]);
	existeDirectorio(path ,1);
	CS_LOG_INFO("Se creo el restaurante %s", consulta[1]);
	string_append(&path, "/info.AFIP");

	char* escritura = string_from_format(
			"CANTIDAD_COCINEROS=%s\n"
			"POSICION=%s\n"
			"AFINIDAD_COCINEROS=%s\n"
			"PLATOS=%s\n"
			"PRECIO_PLATOS=%s\n"
			"CANTIDAD_HORNOS=%s\n",
			consulta[2], consulta[3], consulta[4], consulta[5], consulta[6], consulta[7]
	);

	char* aux = string_from_format("Restaurante %s",consulta[1]);
	char* infoRes = string_from_format(
			"SIZE=%d\n"
			"INITIAL_BLOCK=%d\n",
			strlen(escritura), escribirBloques(escritura, aux)
	);
	free(aux);

	FILE* fd = fopen(path, "wt");
	fwrite(infoRes, strlen(infoRes), 1, fd);
	fclose(fd);
	CS_LOG_INFO("Se creo el archivo \"info.AFIP\" para el restaurant %s", consulta[1]);

	free(infoRes);
	free(escritura);
	free(path);
}

void crearReceta(char** consulta){ // LISTO
	char* path = string_from_format("%s/Files/Recetas/%s.AFIP", puntoMontaje, consulta[1]);
	char* escritura = string_from_format("PASOS=%s\nTIEMPO_PASOS=%s\n", consulta[2], consulta[3]);

	char* aux = string_from_format("Receta %s",consulta[1]);
	char* infoRes = string_from_format(
		"SIZE=%d\n"
		"INITIAL_BLOCK=%d\n",
		strlen(escritura), escribirBloques(escritura, aux)
	);
	free(aux);

	FILE* fd = fopen(path, "wt");
	fwrite(infoRes, strlen(infoRes), 1, fd);
	fclose(fd);
	CS_LOG_INFO("Se creo el archivo \"%s.AFIP\"", consulta[1]);

	free(infoRes);
	free(path);
	free(escritura);

}

// --------------------- MANEJO BITMAP --------------------- //

int obtenerYEscribirProximoDisponible(char* aQuien){
	char* path = string_new();

	string_append(&path, puntoMontaje);
	string_append(&path, "/Metadata/Bitmap.bin");

	sem_wait(&bitmapSem);
	int bitmapFile = open(path, O_CREAT | O_RDWR, 0664);

	void* punteroABitmap = mmap(NULL, cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, bitmapFile, 0);

	t_bitarray* bitmap = bitarray_create_with_mode((char*)punteroABitmap, cantidadBloques/8, MSB_FIRST);

	for(int i=1; i<=cantidadBloques; i++){
		if(bitarray_test_bit(bitmap, i) == 0){
			bitarray_set_bit(bitmap ,i);
			CS_LOG_INFO("Se asigno el bloque %d al %s", i, aQuien);
			msync(punteroABitmap ,cantidadBloques/8 ,0);
			close(bitmapFile);
			sem_post(&bitmapSem);
			free(path);
			bitarray_destroy(bitmap);
			return i;
		}
	}
	close(bitmapFile);
	sem_post(&bitmapSem);
	bitarray_destroy(bitmap);
	free(path);
	CS_LOG_TRACE("BITMAP LLENO");
	return 0;
}

void eliminarBit(int index){
	char* path = string_new();

	string_append(&path, puntoMontaje);
	string_append(&path, "/Metadata/Bitmap.bin");

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

t_rta_obt_rec* cs_string_to_receta(char* string){
	t_rta_obt_rec* receta;
	char* path = string_from_format("%s/ProvisorioRec.AFIP", puntoMontaje);
	FILE* f = fopen(path, "wrb");
	fwrite(string, 1, strlen(string), f);
	fclose(f);
	t_config* conf = config_create(path);
	char* pasos = config_get_string_value(conf, "PASOS");
	char* tiempos = config_get_string_value(conf, "TIEMPO_PASOS");
	receta = cs_rta_obtener_receta_create(pasos, tiempos);
	config_destroy(conf);
	remove(path);
	free(path);
	return receta;
}

// --------------------- PEDIDO --------------------- //

t_rta_obt_ped* leerPedido(char* string){ // CHEQUEAR ORDEN CREATE
	t_rta_obt_ped* pedido;
	char* path = string_from_format("%s/ProvisorioPed.AFIP", puntoMontaje);
	FILE* f = fopen(path, "wrb");
	fwrite(string, 1, strlen(string), f);
	fclose(f);
	t_config* conf = config_create(path);
	e_estado_ped estado = cs_string_to_est_ped(config_get_string_value(conf, "ESTADO_PEDIDO"));
	char* platos = config_get_string_value(conf, "LISTA_PLATOS");
	char* platos_listos = config_get_string_value(conf, "CANTIDAD_LISTA");
	char* cantidad = config_get_string_value(conf, "CANTIDAD_PLATOS");
	pedido = cs_rta_obtener_ped_create(estado, platos, platos_listos, cantidad);
	config_destroy(conf);
	remove(path);
	free(path);
	return pedido;
}

int existePedido(int idPedido, char* nombreRestaurante){
	char* path = string_from_format("%s/Files/Restaurantes/%s/Pedido%d", puntoMontaje, nombreRestaurante,idPedido);
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
	char* path = string_from_format("%s/Files/Restaurantes/%s/Pedido%d", puntoMontaje, nombreRestaurante,idPedido);
	int file = open(path, O_CREAT | O_RDWR, 0664);
	lseek(file, 0, SEEK_END);
	write(file, infoPedido, strlen(infoPedido));
	close(file);
	CS_LOG_INFO("Escribi el Info del pedido %d para el restaurante %s", idPedido, nombreRestaurante);
	free(path);
	free(infoPedido);
}

int obtenerBlockInicialPedido(int idPedido, char* restaurante){
	char* path = string_from_format("%s/Files/Restaurantes/%s/Pedido%d", puntoMontaje, restaurante,idPedido);
	t_config* conf = config_create(path);
	int bloque = config_get_int_value(conf, "INITIAL_BLOCK");
	config_destroy(conf);
	free(path);
	return bloque;
}

int obtenerTamanioPedido(int idPedido, char* restaurante){
	char* path = string_from_format("%s/Files/Restaurantes/%s/Pedido%d", puntoMontaje, restaurante,idPedido);
	t_config* conf = config_create(path);
	int size = config_get_int_value(conf, "SIZE");
	config_destroy(conf);
	free(path);
	return size;
}

e_estado_ped cs_string_to_est_ped(char* lectura){
	if(!strcmp(lectura, "Pendiente")){
		return PEDIDO_PENDIENTE;
	}
	if(!strcmp(lectura, "Confirmado")){
		return PEDIDO_CONFIRMADO;
	}
	return PEDIDO_TERMINADO;
}

int estaEnEstado(char* lectura, e_estado_ped estado){
	char* estadoEnString = string_duplicate((char*)cs_enum_estado_pedido_to_str(estado));
	char* renglon = string_from_format("ESTADO_PEDIDO=%s", estadoEnString);
	int resultado = string_contains(lectura, renglon);
	free(estadoEnString);
	free(renglon);
	return resultado;
}

char* cs_est_ped_to_string(e_estado_ped estado){
	if(estado == PEDIDO_PENDIENTE){
		return "Pendiente";
	}
	if(estado == PEDIDO_CONFIRMADO){
		return "Confirmado";
	}
	return "Terminado";
}

// --------------------- MODIFICAR STRINGS --------------------- //

char* agregarCantPlatos(char* escrituraVieja, t_consulta* consulta){
	char* respuesta;
		char* path = string_from_format("%s/Files/Restaurantes/%s/PedidoAux%d.AFIP",puntoMontaje, consulta->restaurante, consulta->pedido_id);
		FILE* f = fopen(path, "wrb");
		fwrite(escrituraVieja, 1, strlen(escrituraVieja), f);
		fclose(f);
		t_config* conf = config_create(path);
		char** platos = config_get_array_value(conf, "LISTA_PLATOS");
		int i=0;
		int posicion = -1;

		while(platos[i]!=NULL){
			if(!strcmp(platos[i], consulta->comida)){
				posicion = i;
			}
			i++;
		}
		i=0;
		char** cantidad = config_get_array_value(conf, "CANTIDAD_LISTA");

		char* aux = cantidad[posicion];
		cantidad[posicion] = string_itoa(atoi(aux) + consulta->cantidad);
		free(aux);

		aux = cs_string_array_to_string(cantidad);
		config_set_value(conf, "CANTIDAD_LISTA", aux);
		free(aux);

		config_save(conf);
		config_destroy(conf);
		int tamanio = tamanioArchivo(path);
		f = fopen(path, "r");
		respuesta = malloc(tamanio);
		fread(respuesta, 1, tamanio, f);
		fclose(f);
		remove(path);
		free(platos);
		liberar_lista(cantidad);
		free(path);
		return respuesta;
}

char* agregarPlato(char* escrituraVieja, t_consulta* consulta){ // LISTO
	char* respuesta;
	char* leido;
	char* nuevo;
	char* aux;
	char* path = string_from_format("%s/Files/Restaurantes/%s/PedidoAux%d.AFIP",puntoMontaje, consulta->restaurante, consulta->pedido_id);
	FILE* f = fopen(path, "wrb");
	fwrite(escrituraVieja, 1, strlen(escrituraVieja), f);
	fclose(f);
	t_config* conf = config_create(path);
	leido = config_get_string_value(conf, "LISTA_PLATOS");
	if(strlen(leido)==2){
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s%s]",
				aux, consulta->comida);
		free(aux);
	} else {
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s,%s]",
			aux, consulta->comida);
		free(aux);
	}
	config_set_value(conf, "LISTA_PLATOS", nuevo);
	free(nuevo);
	leido = config_get_string_value(conf, "CANTIDAD_LISTA");
	if(strlen(leido)==2){
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s%d]",
					aux, 0);
		free(aux);
	} else {
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s,%d]",
			aux, 0);
		free(aux);
	}
	config_set_value(conf, "CANTIDAD_LISTA", nuevo);
	free(nuevo);
	leido = config_get_string_value(conf, "CANTIDAD_PLATOS");
	if(strlen(leido)==2){
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s%d]",
						aux, consulta->cantidad);
		free(aux);
	} else {
		aux = string_substring_until(leido, strlen(leido) -1);
		nuevo = string_from_format("%s,%d]",
				aux, consulta->cantidad);
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
	char* respuesta;
	char* path = string_from_format("%s/Files/Restaurantes/%s/PedidoAux%d.AFIP",puntoMontaje, consulta->restaurante, consulta->pedido_id);
	FILE* f = fopen(path, "wrb");
	fwrite(escrituraVieja, 1, strlen(escrituraVieja), f);
	fclose(f);
	t_config* conf = config_create(path);
	char** platos = config_get_array_value(conf, "LISTA_PLATOS");
	int i=0;
	int posicion = -1;

	while(platos[i]!=NULL){
		if(!strcmp(platos[i], consulta->comida)){
			posicion = i;
		}
		i++;
	}
	i=0;
	char** cantidad = config_get_array_value(conf, "CANTIDAD_LISTA");
	char* aux = cantidad[posicion];
	cantidad[posicion] = string_itoa(atoi(aux) + 1);
	free(aux);

	aux = cs_string_array_to_string(cantidad);
	config_set_value(conf, "CANTIDAD_LISTA", aux);
	free(aux);

	config_save(conf);
	config_destroy(conf);
	int tamanio = tamanioArchivo(path);
	f = fopen(path, "r");
	respuesta = malloc(tamanio);
	fread(respuesta, 1, tamanio, f);
	fclose(f);
	remove(path);
	free(platos);
	liberar_lista(cantidad);
	free(path);
	return respuesta;
}

char* cambiarEstadoPedidoA(char* lectura, t_consulta* consulta, e_estado_ped estado){ // LISTO
	char* respuesta;
	char* path = string_from_format("%s/Files/Restaurantes/%s/PedidoAux%d.AFIP",puntoMontaje, consulta->restaurante, consulta->pedido_id);
	FILE* f = fopen(path, "wrb");
	fwrite(lectura, 1, strlen(lectura), f);
	fclose(f);
	t_config* conf = config_create(path);
	config_get_string_value(conf, "ESTADO_PEDIDO");
	config_set_value(conf, "ESTADO_PEDIDO", cs_est_ped_to_string(estado));
	config_save(conf);
	config_destroy(conf);
	int tamanio = tamanioArchivo(path);
	f = fopen(path, "r");
	respuesta = malloc(tamanio);
	fread(respuesta, 1, tamanio, f);
	fclose(f);
	remove(path);
	free(path);
	return respuesta;
}

// --------------------- RETOCAR BLOQUES --------------------- //

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
	char* path = string_from_format("%s/Blocks/%d.AFIP", puntoMontaje, bloque);
	FILE* f = fopen(path, "wrb");
	fclose(f);
}

// --------------------- RESTAURANTE --------------------- //

char* obtenerPlatos(char* lectura){
	char** lecturaAux = string_n_split(lectura, 7, "\n");
	char* textoAux = string_duplicate(lecturaAux[3]);
	char** segundoAux = string_split(textoAux, "=");
	char* retorno = string_duplicate(segundoAux[1]);
	free(textoAux);
	liberar_lista(lecturaAux);
	liberar_lista(segundoAux);
	return retorno;
}

char* obtenerPathRestaurante(char* nombreRestaurante){
	char* path = string_new();
	string_append(&path, puntoMontaje);
	string_append(&path, "/Files/Restaurantes/");
	string_append(&path, nombreRestaurante);
	return path;
}

uint32_t obtenerCantidadPedidos(char* nombreRestaurante){
	uint32_t cant = 0;
	char* path = obtenerPathRestaurante(nombreRestaurante);
	char* bash_cmd = string_from_format("ls %s | wc -l", path);
	char* buffer = malloc(1000);
	FILE *pipe;
	int len;
	pipe = popen(bash_cmd, "r");
	if (NULL == pipe) {
	    perror("pipe");
	    exit(1);
	}
	fgets(buffer, 1000, pipe);
	len = strlen(buffer);
	buffer[len-1] = '\0';
	cant = atoi(buffer);
	pclose(pipe);
	free(bash_cmd);
	free(buffer);
	free(path);
	return cant - 1;
}

t_rta_obt_rest* cs_string_to_restaurante(char* string, uint32_t cant_pedidos){
	t_rta_obt_rest* restaurante;
	char* path = string_from_format("%s/Provisorio.AFIP", puntoMontaje);
	FILE* f = fopen(path, "wrb");
	fwrite(string, 1, strlen(string), f);
	fclose(f);
	t_config* conf = config_create(path);
	uint32_t cant_cocineros = config_get_int_value(conf, "CANTIDAD_COCINEROS");
	char* afinidades = config_get_string_value(conf, "AFINIDAD_COCINEROS");
	char* comidas = config_get_string_value(conf, "PLATOS");
	char* precios = config_get_string_value(conf, "PRECIO_PLATOS");
	char** auxPos = config_get_array_value(conf, "POSICION");
	t_pos posicion = cs_string_array_to_pos(auxPos);
	liberar_lista(auxPos);
	uint32_t cant_hornos = config_get_int_value(conf, "CANTIDAD_HORNOS");
	restaurante = cs_rta_obtener_rest_create(cant_cocineros, afinidades, comidas, precios, posicion, cant_hornos, cant_pedidos);
	config_destroy(conf);
	remove(path);
	free(path);
	return restaurante;
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

int cantidadDeBloques(char** bloques){
	int i = 0;
	while(bloques[i]!=NULL){
		i++;
	}
	return i;
}

int tamanioDeBloque(char* bloque){
	char* path = string_new();
	string_append(&path, puntoMontaje);
	string_append(&path, "/TALL_GRASS/Blocks/");
	string_append(&path, bloque);
	string_append(&path, ".bin");

	FILE* fd = fopen(path, "rb");
	fseek(fd, 0, SEEK_END);
	int tamanio = ftell(fd);

	fclose(fd);
	free(path);
	return tamanio;
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
