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
	printf("Ya existe el FL");
	}

	free(metadata);
	free(bitmap);
}


void crearMetadata(char* pathOrigin){
	FILE* metadata;
	char* path = string_duplicate(pathOrigin);

	string_append(&path,"/Metadata");
	char* pathMetadata = string_duplicate(path);
	mkdir(path, 0777);

	CS_LOG_INFO("Agrego directorio Metadata");

	string_append(&path, "/Metadata.");
	string_append(&path, magicNumber);
	metadata = fopen(path, "wrb");
	fclose(metadata);
	t_config* md = config_create(path);
	char* tamanio = string_itoa(tamanioBloque);
	char* cantidad = string_itoa(cantidadBloques);
	config_set_value(md, "BLOCK_SIZE", tamanio);
	config_set_value(md, "BLOCKS", cantidad);
	config_set_value(md, "MAGIC_NUMBER", magicNumber);
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
	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}

e_opcode guardarPedido(t_consulta* consulta){
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

t_rta_obt_ped* obtenerPedido(t_consulta* consulta){ // Esta bien esto?
	t_rta_obt_ped* respuesta = malloc(sizeof(t_rta_obt_ped));
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){

	} else {
		CS_LOG_ERROR("No existe el Restaurant %s", consulta->restaurante);
	}
	free(path);
	return respuesta;
}
//e_opcode+t_rta_cons_ped obtener_pedido(t_consulta*);

t_rta_obt_rest* obtenerRestaurante(t_consulta* consulta){
	t_rta_obt_rest* respuesta = malloc(sizeof(t_rta_obt_rest));
	char* path = obtenerPathRestaurante(consulta->restaurante);
	if(existeDirectorio(path, 0)){

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
// ----------------- MENSAJES COMPONENTES ------------------ //
// --------------------------------------------------------- //

char* crearRestaurante(char** consulta){
	char* respuesta = string_new();
	return respuesta;
}

char* crearReceta(char** consulta){
	char* respuesta = string_new();
	return respuesta;
}

// --------------------- MANEJO BITMAP --------------------- //

int obtenerYEscribirProximoDisponible(){
	char* path = string_new();

	string_append(&path, puntoMontaje);
	string_append(&path, "/Metadata/Bitmap.bin");

	sem_wait(&bitmapSem);
	int bitmapFile = open(path, O_CREAT | O_RDWR, 0664);

	void* punteroABitmap = mmap(NULL, cantidadBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, bitmapFile, 0);

	t_bitarray* bitmap = bitarray_create_with_mode((char*)punteroABitmap, cantidadBloques/8, MSB_FIRST);

	for(int i=0; i<cantidadBloques; i++){
		if(bitarray_test_bit(bitmap, i) == 0){
			bitarray_set_bit(bitmap ,i);
			CS_LOG_INFO("Se asigno el bloque %d", i);
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
	char* cantBloques;

	string_append(&path, puntoMontaje);
	string_append(&path, "/TALL_GRASS/Metadata");

	cantBloques = string_duplicate(path);
	string_append(&cantBloques, "/Metadata.bin");
	t_config* md = config_create(cantBloques);

	int cantidadDeBloques = config_get_int_value(md, "BLOCKS");

	sem_wait(&bitmapSem);
	string_append(&path, "/Bitmap.bin");
	int bitmapFile = open(path, O_CREAT | O_RDWR, 0664);

	void* punteroABitmap = mmap(NULL, cantidadDeBloques/8, PROT_READ | PROT_WRITE, MAP_SHARED, bitmapFile, 0);

	t_bitarray* bitmap = bitarray_create_with_mode((char*)punteroABitmap, cantidadDeBloques/8, MSB_FIRST);

	bitarray_clean_bit(bitmap, index);
	CS_LOG_INFO("Se desasigno el bloque %d", index );

	msync(punteroABitmap ,cantidadDeBloques/8 ,0);

	close(bitmapFile);
	sem_post(&bitmapSem);
	bitarray_destroy(bitmap);

	config_destroy(md);
	free(cantBloques);
	free(path);
}

// --------------------- RECETA --------------------- //

t_rta_obt_rec* leerReceta(char* nombre){
	t_rta_obt_rec* receta = malloc(sizeof(t_rta_obt_rec));
	char* path = string_new();
	string_append(&path, puntoMontaje);
	string_append(&path, "/Files/Recetas/");
	string_append(&path, nombre);
	string_append(&path, ".AFIP");
	if(mkdir(path, 0777)){
		//LOGICA
		free(path);
		return receta;
	} else {
		return NULL;
	}
}

// --------------------- RESTAURANTE --------------------- //

char* obtenerPathRestaurante(char* nombreRestaurante){
	char* path = string_new();
	string_append(&path, puntoMontaje);
	string_append(&path, "/Files/Restaurantes/");
	string_append(&path, nombreRestaurante);
	return path;
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
	int contador = 0;
	while(lista[contador] != NULL){
			free(lista[contador]);
			contador++;
	}
	free(lista);
}
