#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "ERROR. Solo debe aparecer un argumento."
    exit 1
fi

file=$1
if [ -a file ]; then
	echo "ERROR. ya existe el fichero"
	exit 1
fi

echo -n "012345" > f1.txt
echo -n "ABCDEF" > f2.txt

./mytar -c -f $1 f1.txt f2.txt
./mytar -e -f $1

if [ -d tmp ]
 then
    echo "Borrando directorio tmp..."
    rm -r tmp
fi

mkdir tmp
cd tmp
./../mytar -x -f ./../_cifrado_$1

cat f1.txt
cat f2.txt

exit 0


