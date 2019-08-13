Radarmulti:	SO2_TP2_multiprocesador_FrancoRivero.c
	gcc	SO2_TP2_multiprocesador_FrancoRivero.c -Werror -Wall -pedantic -o Radarmulti -fopenmp -lm

clean:
	rm	-f	*.o	Radarmulti
