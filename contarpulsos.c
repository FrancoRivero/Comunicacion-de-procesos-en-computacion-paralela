#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <complex.h>



int main(int argc, char *argv[])
{
	/*Declaracion de variables*/
	float pulsos = 0;
	float muestras = 0;
	int contador = 0;
	FILE * binfile = fopen ("franco", "r");

	while(!feof(binfile))
	{
		fscanf(binfile,"%f",&muestras);
		printf("%f\n",muestras);
		pulsos += muestras;
		contador ++;
	}
	printf("El promodio entre %f y %u es: %f\n",pulsos,contador,pulsos/contador*1000000);
	fclose(binfile);
	return 0;
}