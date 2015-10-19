#! /bin/bash
if [ ! -x mytar ]
 then
    echo "mytar no encontrado."
    exit 1
fi

if [ -d tmp ]
 then
    echo "Borrando directorio tmp..."
    rm -r tmp
fi

echo "Creando directorio tmp..."
mkdir tmp
cd ./tmp

echo "Creando archivo file1.txt"
echo "Hello world!" > file1.txt

echo "Creando archivo file2.txt"
head -n 10 /etc/passwd > file2.txt

echo "Creando archivo file3.dat"
head -c 10 /dev/random > file3.dat

echo "Creando fichero filetar.mtar"
./../mytar -c -f filetar.mtar file1.txt file2.txt file3.dat

mkdir out
cp filetar.mtar ./out/

cd ./out
echo "Extrayendo contenido de filetar.mtar en ./out/"
./../../mytar -x -f filetar.mtar

echo "Comprobando archivo extraido file1.txt"
DIFF=$(diff file1.txt ./../file1.txt)
if [ "$DIFF" != "" ]; then
    cd ../..
    echo "Archivo extraido file1.txt no coincide con el original"
    echo diff -q file1.txt ./file1.txt
    exit 1
fi

echo "Comprobando archivo extraido file2.txt"
DIFF=$(diff file2.txt ./../file2.txt)
if [ "$DIFF" != "" ]; then
    cd ../..
    echo "Archivo extraido file2.txt no coincide con el original"
    echo diff -q file2.txt ./file2.txt
    exit 1
fi

echo "Comprobando archivo extraido file3.dat"
DIFF=$(diff file3.dat ./../file3.dat)
if [ "$DIFF" != "" ]; then
    cd ../..
    echo "Archivo extraido file3.dat no coincide con el original"
    echo diff -q file3.dat ./file3.dat
    exit 1
fi

echo "Correct"
exit 0




