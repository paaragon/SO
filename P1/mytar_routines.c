#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy -- Esto para que?
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int copynFile(FILE * origin, FILE * destination, int nBytes) {
	
	if(origin == NULL || destination == NULL) return -1;
	
	int i = 0;
	const int bufSize = 1024;
	char* buf = (char*) malloc(bufSize*sizeof(unsigned char));
	
	while((i < nBytes) &&  fread(buf, sizeof(unsigned char), bufSize, origin) > 0){
		fwrite(buf, sizeof(unsigned char)*strlen(buf), 1, destination);
		i+=strlen(buf)*sizeof(unsigned char);
	}
	
	return i;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * buf: parameter to return the read string. Buf is a
 * string passed by reference. 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly "built" in memory, return the starting 
 * address of the string (pointer returned by malloc()) in the buf parameter
 * (*buf)=<address>;
 * 
 * Returns: 0 if success, -1 if error
 */
int loadstr(FILE * file, char **buf) {
	
	if(file == NULL) return -1;
	
	int i = 0;
	char c = ' ';
	
	// Contamos el tamaño de la cadena a leer
	do{
		fread(&c, sizeof(char), 1, file);
		i++;
	}while(!feof(file) && c != '\0');
	
	// Volvemos i caracteres hacia atras en el fichero
	fseek(file, i*-1, SEEK_CUR);
	
	// Reservamos i Bytes de memoria para buf
	char* p = (char*) malloc(i*sizeof(unsigned char));
	// Escribimos en buf
	i = 0;
	
	fread(&c, sizeof(char), 1, file);
	do{
		p[i] = c;
		i++; 
	}while(fread(&c, sizeof(char), 1, file) != 0 && c != '\0');
		
	*buf = p;
	return 0;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * header: output parameter. It is used to return the starting memory address
 * of an array that contains the (name,size) pairs read from the tar file
 * nFiles: output parameter. Used to return the number of files stored in
 * the tarball archive (first 4 bytes of the header)
 *
 * On success it returns EXIT_SUCCESS. Upon failure, EXIT_FAILURE is returned.
 * (both macros are defined in stdlib.h).
 */
int readHeader(FILE * tarFile, stHeaderEntry ** header, int *nFiles) {
	
	/*if(tarFile == NULL) return EXIT_FAILURE;
	
	int i;
	char *buf;
	
	// Leemos el numero de ficheros y lo guardamos en nFiles
	fread(nFiles,sizeof(int),1,tarFile);
	
	// Reservamos memoria para header (nFiles * tamaño de puntero)
	
	// Leemos el nombre del fichero y su tamaño
	for(i = 0; i < *nFiles; i++){
		loadstr(tarFile, &buf);
		header[i].name = buf;
		fread(&header[i].size,sizeof(int),1,tarFile);
	}*/
	
	return EXIT_SUCCESS;
}
/** Calculates the size of the header of the tarball 
 */
int getHeaderSize(char *fileNames[],int nFiles){
	int headerSize = 0;
	int i;
	
	// Sumo en headerSize los tamanos de los nombres de los archivos
	for(i = 0; i < nFiles; i++){
		headerSize += strlen(fileNames[i]) + 1;
	}
	
	// Sumo en headerSize los Bytes necesarios para almacenar los tamanos de los archivos
	headerSize += nFiles*sizeof(int);
	
	// Sumo el tamano del entero que indica el numero de archivos en la cabecera
	headerSize += sizeof(int);
	
	return headerSize;
}

/** Writes one file in the tarball file
 *  
 * On success it returns the header entry of the file
 */
stHeaderEntry writeFileInTar(char* fileName, FILE *file){
	
	stHeaderEntry headerEntry;
	headerEntry.name = NULL;
	
	FILE *f = fopen(fileName, "r");
	
	if(f == NULL) return headerEntry;
	
	fseek(f, 0, SEEK_END);
	int fSize = ftell(f);
	rewind(f);
	
	int size = copynFile(f, file, fSize);
	
	headerEntry.name = fileName;
	headerEntry.size = size;
	
	return headerEntry;
}

int writeHeaderInTar(stHeaderEntry* header, FILE *file, int nFiles){
	
	fseek(file, 0, SEEK_SET);
	fwrite(&nFiles, sizeof(int), 1, file);
	
	int i;
	
	for(i = 0; i < nFiles; i++){
		
		fwrite(header[i].name, sizeof(unsigned char)*strlen(header[i].name)+1, 1, file);
		fwrite(&header[i].size, sizeof(int), 1, file);
	}
	
	return 0;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int createTar(int nFiles, char *fileNames[], char tarName[]) {
	
	if(fileNames == NULL) return EXIT_FAILURE;
	
	FILE * file;
	int headerSize;
	int i = 0;
	stHeaderEntry* header = (stHeaderEntry*)malloc(nFiles*sizeof(stHeaderEntry));
	
	//Obtenemos el tamaño en Bytes de la cabecera
	headerSize = getHeaderSize(fileNames, nFiles);
	
	//Abrimos el archivo tarball
	file = fopen(tarName, "w");
	
	if(file == NULL) return EXIT_FAILURE;
	
	// Reservamos espacio para la cabecera
	fseek(file, headerSize, SEEK_SET);
	
	//Añadimos cada archivo al tar
	for(i = 0; i < nFiles; i++){
		header[i] = writeFileInTar(fileNames[i], file);
		if(header[i].name == NULL){
			printf("\nERROR: archivo %s no encontrado\n", fileNames[i]);
			return EXIT_FAILURE;
		}
	}
	
	writeHeaderInTar(header, file, nFiles);
	
	fclose(file);
	
	printf("\nFichero mytar creado con exito.\n\n");
	
	return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int extractTar(char tarName[]) {
	
	FILE *tarFile = fopen(tarName, "r");
	unsigned char* c;
	
	int nFiles = 0;
	fread(&nFiles, sizeof(int), 1, tarFile);
	
	stHeaderEntry* header = (stHeaderEntry*)malloc(nFiles*sizeof(stHeaderEntry));
	
	int i = 0;
	for(i = 0; i < nFiles; i++){
		loadstr(tarFile, &header[i].name);
		fread(&header[i].size, sizeof(int), 1, tarFile);
	}
	
	for(i = 0; i < nFiles; i++){
		
		FILE *destination = fopen(header[i].name, "w");
		
		c = (unsigned char*)malloc(header[i].size);
		fread(c, sizeof(unsigned char) * header[i].size, 1, tarFile);
		fwrite(c, sizeof(unsigned char) * header[i].size, 1, destination);
		
		fclose(destination);
		
	}
	
	fclose(tarFile);
	
	printf("\nFichero mytar extraido con exito.\n\n");
	
	return EXIT_SUCCESS;
}

int getKey(char tarName[]){
	
	int i = 0;
	int key = 0;
	for(i = 0; i < strlen(tarName); i++){
		key += tarName[i];
	}
	
	return key % 256;
}

int cifrarTar(char tarName[]){
	
	char destName[] = "_cifrado_";
	strcat(destName, tarName);
	
	FILE* tar = fopen(tarName, "r");
	
	if(tar == NULL) return 1;
	
	int nFiles = 0;
	fread(&nFiles, sizeof(int), 1, tar);
	
	stHeaderEntry* header = (stHeaderEntry*)malloc(nFiles*sizeof(stHeaderEntry));
	
	int i = 0;
	for(i = 0; i < nFiles; i++){
		loadstr(tar, &header[i].name);
		fread(&header[i].size, sizeof(int), 1, tar);
	}
	
	FILE* dest = fopen(destName, "w");
	writeHeaderInTar(header, dest, nFiles);
	
	unsigned char* c;
	int key = 0;
	int j = 0;
	for(i = 0; i < nFiles; i++){
		
		key = getKey(header[i].name);
		
		c = (unsigned char*)malloc(header[i].size);
		fread(c, sizeof(unsigned char) * header[i].size, 1, tar);
		
		for(j = 0; j < sizeof(unsigned char) * header[i].size; j++){
			c[j] += key;
		}
		fwrite(c, sizeof(unsigned char) * header[i].size, 1, dest);		
	}
	
	fclose(dest);
	fclose(tar);	
	
	return 0;
}

int descifrarTar(char tarName[]){
		
	char destName[] = "_descifrado_";
	strcat(destName, tarName);
	
	FILE* tar = fopen(tarName, "r");
	
	if(tar == NULL) return 1;
	
	int nFiles = 0;
	fread(&nFiles, sizeof(int), 1, tar);
	
	stHeaderEntry* header = (stHeaderEntry*)malloc(nFiles*sizeof(stHeaderEntry));
	
	int i = 0;
	for(i = 0; i < nFiles; i++){
		loadstr(tar, &header[i].name);
		fread(&header[i].size, sizeof(int), 1, tar);
	}
	
	FILE* dest = fopen(destName, "w");
	writeHeaderInTar(header, dest, nFiles);
	
	unsigned char* c;
	int key = 0;
	int j = 0;
	for(i = 0; i < nFiles; i++){
		
		key = getKey(header[i].name);
		
		c = (unsigned char*)malloc(header[i].size);
		fread(c, sizeof(unsigned char) * header[i].size, 1, tar);
		
		for(j = 0; j < sizeof(unsigned char) * header[i].size; j++){
			c[j] -= key;
		}
		fwrite(c, sizeof(unsigned char) * header[i].size, 1, dest);		
	}
	
	fclose(dest);
	fclose(tar);	
	
	return 0;
}

