#!/bin/bash

# a) Copie dos ficheros de texto que ocupen más de un boque (por ejemplo fuseLib.c y
#    myFS.h) a nuestro SF y a un directorio temporal, por ejemplo ./temp

if test -e ./temp; then
	rm -r ./temp
fi

mkdir ./temp

cp ./src/fuseLib.c  ./mount-point
cp ./src/myFS.h     ./mount-point
cp ./src/fuseLib.c  ./temp
cp ./src/myFS.h     ./temp

# b) Audite el disco y haga un diff entre los ficheros originales y los copiados en el SF
#    Trunque el primer fichero (man truncate) en copiasTemporales y en nuestro SF de
#    manera que ocupe ocupe un bloque de datos menos.

./my-fsck-static-64 virtual-disk

diff ./temp/fuseLib.c ./mount-point/fuseLib.c
diff ./temp/myFS.h ./mount-point/myFS.h

truncate -s -1 -o ./temp/fuseLib.c
truncate -s -1 -o ./mount-point/fuseLib.c

# c) Audite el disco y haga un diff entre el fichero original y el truncado.

./my-fsck-static-64 virtual-disk

diff ./temp/fuseLib.c ./mount-point/fuseLib.c

# d) Copie un tercer fichero de texto a nuestro SF

cp ./src/MyFileSystem.c ./mount-point

# e) Audite el disco y haga un diff entre el fichero original y el copiado en el SF

./my-fsck-static-64 virtual-disk

diff ./temp/fuseLib.c ./mount-point/fuseLib.c

# f) Trunque el segundo fichero en copiasTemporales y en nuestro SF haciendo que ocupe
#    algún bloque de datos más.

truncate -s +1 -o ./temp/myFS.h
truncate -s +1 -o ./mount-point/myFS.h

# g) Audite el disco y haga un diff entre el fichero original y el truncado.

./my-fsck-static-64 virtual-disk

diff ./temp/myFS.h ./mount-point/myFS.h

