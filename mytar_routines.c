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
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int copynFile(FILE * origin, FILE * destination, int nBytes) {
	
	if(origin == NULL || destination == NULL) return -1;
	
	char c;
	int i = 0;
		
	while((i < nBytes) && ((c = fgetc(origin) != EOF )){
		fputc(c, destination);
		i++;
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
	char c = '';
	
	// Contamos el tamaño de la cadena a leer
	do{
		c = fgetc(file);
		i++;
	}while(!feof(file) && c != '\0');
	
	// Volvemos i caracteres hacia atras en el fichero
	fseek(file, i*-1, SEEK_CUR);
	
	// Reservamos i Bytes de memoria para buf
	buf = (char*) malloc(i);
	
	// Escribimos en buf
	i = 0;
	do{
		c = fgetc(file);
		buf[i] = c;
		i++;
	}while(!feof(file) && c != '\0');
	
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
	
	if(tarFile == NULL) return EXIT_FAILURE;
	
	int i;
	char *buf;
	
	// Leemos el numero de ficheros y lo guardamos en nFiles
	fread(nFiles,sizeof(int),1,tarFile);
	
	// Reservamos memoria para header (nFiles * tamaño de puntero)
	stHeaderEntry* h = (stHeaderEntry*)malloc(*nFiles*sizeof(stHeaderEntry));
	
	// Leemos el nombre del fichero y su tamaño
	for(i = 0; i < *nFiles; i++){
		loadstr(tarFile, &buf);
		h[i].name = *buf;
		fread(h[i].size,sizeof(int),1,tarFile);
	}
	
	return EXIT_SUCCESS;
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
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	// Complete the function
	return EXIT_FAILURE;
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
int
extractTar(char tarName[])
{
	// Complete the function
	return EXIT_FAILURE;
}
