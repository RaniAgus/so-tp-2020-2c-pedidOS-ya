#include "filesystem.h"

// --------------------- LEVANTAR FILE SYSTEM --------------------- //

void crearDirectorioAFIP(){
	char* path = string_new();
	string_append(&path,puntoMontaje);

	if(!mkdir(path, 0777)){
		log_info(logger, "Genere el directorio para el FL en %s", puntoMontaje);
		crearMetadata(path);
		crearFiles(path);
		crearBlocks(path);
	} else {
	free(path);
	}
}


void crearMetadata(char* pathOrigin){
	FILE* metadata;
	char* path = string_duplicate(pathOrigin);

	string_append(&path,"/Metadata");
	char* pathMetadata = string_duplicate(path);
	mkdir(path, 0777);

	log_info(logger, "Agrego carpeta Metadata");

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

	log_info(logger, "Agrego el Metadata del File System");

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

	log_info(logger, "Agrego carpeta Files");
	log_info(logger, "Agrego carpeta Restaurantes");
	log_info(logger, "Agrego carpeta Recetas");
	free(path);
}


void crearBlocks(char* path){
	string_append(&path, "/Blocks");
	mkdir(path, 0777);
	log_info(logger, "Agrego la carpeta Blocks");
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

	log_info(logger, "Genere el bitmap");
}
