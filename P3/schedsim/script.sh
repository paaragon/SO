#!/bin/bash

# Creamos la carpeta resultados
if test -d ./resultados; then
    rm -r ./resultados
fi
mkdir ./resultados

# Solicitamos datos al usuario

# Ejemplos a simular
echo -n "Ejemplo a simular: "
read example
if [ ! -f $example ] ; then
	while [ ! -f $example ] ; do
		echo -n ">>>Error: el ejemplo debe ser un fichero regular\n"
		echo -n "Ejemplo a simular: "
		read example
	done
fi

# Numero maximo de CPUs
echo -n "Numero maximo CPUs: "
read maxCPUs
if [ $maxCPUs -gt 8 ] ; then
	while [ $maxCPUs -gt 8 ] ; do
		echo -n ">>>Error: Numero maximo de CPUs 8\n"
		echo -n "Numero maximo CPUs: "
		read maxCPUs
	done
fi

# Ejecutamos la simulacion para todos los sched disponibles
for nameSched in RR SJF FCFS PRIO ; do
	i=1	
	while [ $i -le $maxCPUs ] ; do
		echo -n "***** Simulando planificador $nameSched para $i CPUs *****\n"   		
		./schedsim -s $nameSched -i $example -n $i
		j=0
		while [ $j -lt $i ] ; do
    		mv CPU_$j.log resultados/$nameSched-CPUs-$i-CPU-$j.log
			cd ../gantt-gplot
			# Generamos grafico de Gantt		
			./generate_gantt_chart ../schedsim/resultados/$nameSched-CPUs-$i-CPU-$j.log
			cd ../schedsim
			j=$(($j+1))
		 done
	i=$(($i+1))
	done
done

