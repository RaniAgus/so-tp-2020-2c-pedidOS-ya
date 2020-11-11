#include "filesystem.h"

// --------------------- LEVANTAR FILE SYSTEM --------------------- //

void crearDirectorioAFIP(){
	char* path = string_new();
	string_append(&path,puntoMontaje);

	char* metadata = string_duplicate(path);
	string_append(&metadata, "/Metadata/Metadata.AFIP");

	char* bitmap = string_duplicate(path);
	string_append(&bitmap, "/Metadata/Bitmap.bin");

	if(fopen(metadata,"r") == NULL || fopen(bitmap, "r") == NULL){
		mkdir(puntoMontaje, 0777);
		CS_LOG_INFO("Genere el directorio %s", puntoMontaje);
		crearMetadata(path);
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
	FILE* metadata;
	char* path = string_duplicate(pathOrigin);

	string_append(&path,"/Metadata");
	char* pathMetadata = string_duplicate(path);
	existeDirectorio(path, 1);

	CS_LOG_INFO("Agrego directorio Metadata");

	string_append(&path, "/Metadata.AFIP");
	metadata = fopen(path, "wrb");
	fclose(metadata);
	t_config* md = config_create(path);
	char* tamanio = string_itoa(tamanioBloque);
	char* cantidad = string_itoa(cantidadBloques);
	config_set_value(md, "BLOCK_SIZE", tamanio);
	config_set_value(md, "BLOCKS", cantidad);
	config_set_value(md, "MAGIC_NUMBER", "AFIP");
	config_save(md);
	free(tamanio);
	free(cantidad);

	CS_LOG_INFO("Agrego el Metadata del File System");

	generarBitmap(pathMetadata, md);
	config_destroy(md);
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


void generarBitmap(char* pathOrigin, t_config* md){

	char* path = string_duplicate(pathOrigin);
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

t_rta_cons_pl* consultarPlatos(t_consulta* consulta){
	char* path = obtenerPathRestaurante(consulta->restaurante);
	t_rta_cons_pl* respuesta;
	if(existeDirectorio(path, 0)){
		string_append(&path, "/info.AFIP");
		t_config* md = config_create(path);
		char* platos = config_get_string_value(md, "PLATOS");
		respuesta = cs_rta_consultar_pl_create(platos);
		config_destroy(md);
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
		respuesta = NULL;
	}
	free(path);
	return respuesta;
}

e_opcode guardarPedido(t_consulta* consulta){
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
			int primerBloque = escribirBloques(escritura, string_from_format("Pedido %d",consulta->pedido_id));
			if(primerBloque){
				char* infoPedido = string_from_format("SIZE=%d\nINITIAL_BLOCK=%d", strlen(escritura), primerBloque);
				escribirInfoPedido(infoPedido, consulta->pedido_id, consulta->restaurante);
				respuesta = OPCODE_RESPUESTA_OK;
			} else {
				respuesta = OPCODE_RESPUESTA_FAIL;
			}
		}
	} else {
		respuesta = OPCODE_RESPUESTA_FAIL;
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

e_opcode guardarPlato(t_consulta* consulta){
	e_opcode respuesta;
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){

	} else {
		respuesta = OPCODE_RESPUESTA_FAIL;
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

e_opcode confirmarPedido(t_consulta* consulta){
	e_opcode respuesta;
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){

	} else {
		respuesta = OPCODE_RESPUESTA_FAIL;
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

t_rta_obt_ped* obtenerPedido(t_consulta* consulta){
	t_rta_obt_ped* respuesta = malloc(sizeof(t_rta_obt_ped));
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){

	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

t_rta_obt_rest* obtenerRestaurante(t_consulta* consulta){
	t_rta_obt_rest* respuesta = NULL;
	char* path = obtenerPathRestaurante(consulta->restaurante);
	uint32_t cantPedidos = obtenerCantidadPedidos(consulta->restaurante);
	if(existeDirectorio(path, 0)){
		string_append(&path, "/info.AFIP");
		t_config* md = config_create(path);
		uint32_t cantCocineros = config_get_int_value(md, "CANTIDAD_COCINEROS");
		uint32_t cantHornos = config_get_int_value(md, "CANTIDAD_HORNOS");
		char* afinidad = config_get_string_value(md, "AFINIDAD_COCINEROS");
		char* platos = config_get_string_value(md, "PLATOS");
		char* precios = config_get_string_value(md, "PRECIO_PLATOS");
		char** posicion = config_get_array_value(md, "POSICION");
		t_pos pos = cs_string_array_to_pos(posicion);
		liberar_lista(posicion);
		cs_rta_obtener_rest_create(cantCocineros, afinidad, platos, precios, pos, cantHornos, cantPedidos);
		config_destroy(md);
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

e_opcode platoListo(t_consulta* consulta){
	e_opcode respuesta;
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){

	} else {
		respuesta = OPCODE_RESPUESTA_FAIL;
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

t_rta_obt_rec* obtenerReceta(t_consulta* consulta){
	t_rta_obt_rec* respuesta = leerReceta(consulta->comida);
	if(respuesta == NULL){
		CS_LOG_ERROR("No existe el Plato %s", consulta->comida);
	}
	return respuesta;
}



// --------------------------------------------------------- //
// ----------------- MENSAJES CONSOLA ------------------ //
// --------------------------------------------------------- //

void crearRestaurante(char** consulta){
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

	char* infoRes = string_from_format(
			"SIZE=%d\n"
			"INITIAL_BLOCK=%d\n",
			strlen(escritura), escribirBloques(escritura, consulta[1])
	);

	FILE* fd = fopen(path, "wt");
	fwrite(infoRes, strlen(infoRes), 1, fd);
	fclose(fd);
	CS_LOG_INFO("Se creo el archivo \"info.AFIP\" para el restaurant %s", consulta[1]);

	escribirBloques(escritura, string_from_format("Restaurante %s",consulta[1]));

	free(escritura);
	free(path);
}

void crearReceta(char** consulta){
	char* path = string_from_format("%s/Files/Recetas/%s.AFIP", puntoMontaje, consulta[1]);
	char* escritura = string_from_format("PASOS=%s\nTIEMPO_PASOS=%s\n", consulta[2], consulta[3]);

	FILE* fd = fopen(path, "wt");
	fwrite(escritura, strlen(escritura), 1, fd);
	fclose(fd);
	CS_LOG_INFO("Se creo el archivo \"%s.AFIP\"", consulta[1]);

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
	printf("BITMAP LLENO");
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

t_rta_obt_rec* leerReceta(char* nombre){
	t_rta_obt_rec* receta;
	char* path = string_new();
	char* pasos = string_new();
	char* tiempos = string_new();
	string_append(&path, puntoMontaje);
	string_append(&path, "/Files/Recetas/");
	string_append(&path, nombre);
	string_append(&path, ".AFIP");
	if(fopen(path,"r") != NULL){
		t_config* md = config_create(path);
		pasos = config_get_string_value(md,"PASOS");
		tiempos = config_get_string_value(md,"TIEMPO_PASOS");
		receta = cs_rta_obtener_receta_create(pasos, tiempos);
		config_destroy(md);
	} else {
		receta = NULL;
	}
	free(path);
	free(pasos);
	free(tiempos);
	return receta;
}

// --------------------- PEDIDO --------------------- //

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

// --------------------- BLOQUES --------------------- //

int escribirBloques(char* escritura, char* aQuien){
	int primerBloque = 0;
	int puntero = 0;
	int numBloque = obtenerYEscribirProximoDisponible(aQuien);
	int proxBloque = obtenerYEscribirProximoDisponible(aQuien);
	primerBloque = numBloque;
	char* escrituraAux = string_substring(escritura, puntero, tamanioBloque - tamanioReservado);
	puntero = puntero + strlen(escrituraAux);
	string_append(&escrituraAux, "\n");
	string_append(&escrituraAux, string_itoa(proxBloque));
	escribirBloque(escrituraAux, numBloque);
	int cantidadEscrituras = (strlen(escritura) + tamanioBloque - 1)/tamanioBloque;
	for(int i=0; i<cantidadEscrituras; i++){
		free(escrituraAux);
		numBloque = proxBloque;
		if(i+1 != cantidadEscrituras){
			proxBloque = obtenerYEscribirProximoDisponible(aQuien);
			escrituraAux = string_substring(escritura, puntero, tamanioBloque - tamanioReservado);
			puntero = puntero + strlen(escrituraAux);
			string_append(&escrituraAux, "\n");
			string_append(&escrituraAux, string_itoa(proxBloque));
		} else {
			escrituraAux = string_substring_from(escritura, puntero);
		}
		escribirBloque(escrituraAux, numBloque);
	}
	return primerBloque;
}

void escribirBloque(char* escritura, int numBloque){
	char* path = string_from_format("%s/Blocks/%d.AFIP", puntoMontaje, numBloque);
	FILE* fd = fopen(path, "wt");
	fwrite(escritura, strlen(escritura), 1, fd);
	fclose(fd);
	CS_LOG_INFO("Escribi el bloque %d", numBloque);
	free(path);
}

char* leerBloques(int initialBlock){
	char* lectura = string_new();
	char* lecturaAux = string_new();
	int nextBlock;
	char* path = string_from_format("%s/Blocks/%d.AFIP", puntoMontaje, initialBlock);
	FILE* f = fopen(path, "r");
	fseek(f, 0L, SEEK_END);
	int tamanioArchivo = ftell(f);
	int tamanioInicial = tamanioArchivo;
	fseek(f, 0L, SEEK_SET);
	while(tamanioArchivo == tamanioInicial){
		fread(lecturaAux, tamanioBloque, 1, f);
		string_append(&lectura, string_substring_until(lecturaAux, tamanioBloque - tamanioReservado));
		nextBlock = atoi(string_substring_from(lecturaAux, tamanioBloque - tamanioReservado));
		fclose(f);
		path = string_from_format("%s/Blocks/%d.AFIP", puntoMontaje, nextBlock);
		f = fopen(path, "r");
		fseek(f, 0L, SEEK_END);
		tamanioArchivo = ftell(f);
		fseek(f, 0L, SEEK_SET);
	}
	fread(lecturaAux, tamanioArchivo, 1, f);
	string_append(&lectura, string_substring_until(lecturaAux, tamanioArchivo));
	fclose(f);
	return lectura;
}

// --------------------- RESTAURANTE --------------------- //

char* obtenerPathRestaurante(char* nombreRestaurante){
	char* path = string_new();
	string_append(&path, puntoMontaje);
	string_append(&path, "/Files/Restaurantes/");
	string_append(&path, nombreRestaurante);
	return path;
}

uint32_t obtenerCantidadPedidos(char* nombreRestaurante){
	uint32_t cant = 1;
	char* path = obtenerPathRestaurante(nombreRestaurante);
	char* pathAux = string_from_format("%s/Pedido%d.AFIP", path, cant);
	FILE* f = fopen(pathAux, "r");
	while(f != NULL){
		cant++;
		fclose(f);
		free(pathAux);
		pathAux = string_from_format("%s/Pedido%d.AFIP", path, cant);
		f = fopen(pathAux, "r");
	}
	free(path);
	free(pathAux);
	return cant - 1;
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

void liberar_lista(char** lista){
	string_iterate_lines(lista, (void*) free);
	free(lista);
}
