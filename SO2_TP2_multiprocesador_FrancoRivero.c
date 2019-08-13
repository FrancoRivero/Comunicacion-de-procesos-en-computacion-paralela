#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <complex.h>

#define GATES 500
#define PULSOS 800
#define THREADS 2

/*se define la cantidad de pulsos medidos y la cantidad de gates a partir de la informacion inicial del problema */
int main(int argc, char *argv[])
{
	/*Declaracion de variables*/
	int muestras; // sera una variable que guardara las muestras del problema
	int pulsos= 0; //sera una variable que guardara el conteo de en que pulso se esta.
	complex float pulso = 0 + 0*I;
	complex float V[GATES][PULSOS]; //guardara los datos del canal V de forma (gate,pulso)
	complex float H[GATES][PULSOS]; //guardara los datos del canal H de forma (gate,pulso)
	float VC[GATES]; // guardara los datos de la autocorrelacion de cada gate del canal V 
	float HC[GATES]; // guardara los datos de la autocorrelacion de cada gate del canal H
	int mxg= 0,overflow = 0,n,j,g,p,t; //variables auxiliares
	float aux = 0,aux2 = 0,sum_q = 0,sum_i = 0; //variables auxiliares
	FILE * binfile = fopen ("pulsos.iq", "rb");
	FILE * a = fopen("gates.iq","rb");
	/*
		Variables para medir el tiempo
	*/
	double inicio;
	double final;
	double rendimiento;
	
	/*inicializo a las matrices V y H con cero*/
	for(n=0;n<GATES;n++)
	{
		for(j=0;j<PULSOS;j++)
		{
			V[n][j] = 0 + 0*I;
			H[n][j] = 0 + 0*I;
		}
		VC[n] = 0;
		HC[n] = 0;
	}
	/*Recorro la matriz por columnas es decir pulso por pulso*/
	while(pulsos<PULSOS)
	{
		t = 0;
		fread(&muestras,sizeof(unsigned short),1,binfile);
		mxg = muestras/GATES; //cantidad de muestras necesarias para completar un gate
		overflow = muestras - mxg * GATES; //me dice la cantidad de muestras que se descartan
		/*Recorro la matriz por filas es decir gate por gate*/
		
		for (n = 0; n < GATES; n++)
		{
			sum_q =0;
			sum_i = 0;	
			for(j = 0; j<mxg ; j++)
			{
				fread(&aux,sizeof(float),1,binfile); //leo la parte real es decir I
				sum_i+=aux; //sumo las muestras por cada gate
				fread(&aux2,sizeof(float),1,binfile); //leo la parte imaginaria es decir Q
				sum_q+=aux2;
				g++;
			}
			sum_i = sum_i/mxg; //realizo la media aritmetica de la fase del pulso en el canal V
			sum_q = sum_q/mxg; //realizo la media aritmetica de la cuadratura del pulso en el canal V
			pulso = sum_i + sum_q*I; //creo el pulso de forma compleja haciendo que la parte real sea la fase y la parte imaginaria sea la cuadratura en el pulso
			V[n][pulsos] += pulso; //guardo el pulso en la matriz V(gate,pulso)
			sum_i = 0;
			sum_q = 0;
			pulso = 0 + 0*I;
			
			if(g==mxg*GATES)
			{
				for(p=0;p<overflow;p++)
				{	
					fread(&aux2,sizeof(float),1,binfile);
					fread(&aux,sizeof(float),1,binfile);
					t++;
				}
				g = 0;
			}
		}
		/*reinicio las variables para trabajar en el canal H*/
			/*realizo nuevamente el mismo procedimiento*/
		if(t == overflow)
		{
			for (n = 0; n < GATES; n++)
			{
				sum_q =0;
				sum_i = 0;
				
				for(j = 0; j<mxg ; j++)
				{
					fread(&aux,sizeof(float),1,binfile); 
					sum_i+=aux; 
					fread(&aux2,sizeof(float),1,binfile); 
					sum_q+=aux2;
					g++;
				}	
					sum_i = sum_i/mxg;
					sum_q = sum_q/mxg;	
					pulso = sum_i + sum_q*I;
					H[n][pulsos] += pulso;
					sum_i = 0;
					sum_q = 0;
					pulso = 0 + 0*I;
					if(g==mxg*GATES)
					{
						for(p=0;p<overflow;p++)
						{	
							fread(&aux,sizeof(float),1,binfile); 
							fread(&aux2,sizeof(float),1,binfile);
						}
						g = 0;
						
					}
				
			}
			t = 0;
		}
		pulsos++;
	}
	fclose(binfile);
	aux = 0;
	aux2 = 0;
	/* Inicio la cuenta y el paralelismo*/
	omp_set_num_threads(THREADS);	
	inicio = omp_get_wtime();
	/*
		Iniciamos paralelismo
	*/
	#pragma omp parallel for shared(VC,HC) private(aux, aux2, n, j)
	/*Se realiza la formula de autocorrelacion es la suma de los pulso por cada gate, divido a la cantidad de pulsos*/
	for(n=0;n<GATES;n++)
	{	
		for(j=1;j<PULSOS;j++)
		{
			aux += cabsf(V[n][j])*cabsf(conj(V[n-1][j])); //consigo la suma de V(m)*V(m+1)
			aux2 += cabsf(H[n][j])*cabsf(conj(H[n-1][j])); //consigo la suma de V(m)*V(m+1)
		}
		aux = aux/PULSOS; //divido por la cantidad de pulsos para conseguir la autocorrelacion
		aux2 = aux2/PULSOS;
		VC[n] +=aux;
		HC[n] += aux2;
	}
	final = omp_get_wtime();
	rendimiento = (double)(final-inicio);
	printf("Tiempo de calculo de autocorrelacion: %f segundos.\n", rendimiento);
		
	/*
		Guardo los calculos en un archivo binario
	*/
	a= fopen("gates.iq", "wb");
	fwrite(&n,sizeof(int),1,a);
	
	for(n=0;n<GATES;n++)
	{
		fwrite(&VC[n],sizeof(float),1,a);
		fwrite(&HC[n],sizeof(float),1,a);
	}

	fclose(a);
	return 0;
}

