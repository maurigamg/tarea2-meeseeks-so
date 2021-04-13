/*
ITCR, Sede Cartago
Escuela de Computación, IC-6600 Principios de Sistemas Operativos
SO - Tarea 2 - Mr. Meeseeks
Estudiantes:
	Mauricio Gamboa Godínez 2018113173
	José Daniel Macías Reynaud 2018241572
Profesor: Esteban Arias Méndez
Fecha de entrega: 19 de abril, 2021, I Semestre
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

char * createMeeseeks(char * query){
	return NULL;
}



int main(int argc, char **argv){
	char * query;
	char continueExecution = 'S';
	char queryType = 'S';

	while(1){
		printf("\nConsulta textual (T)\n");
		printf("Operacion aritmetica (A)\n");
		printf("Operacion logica (L)\n");
		printf("Ejecutar programa (P)\n");
		printf("Indique el tipo de solicitud: ");
    	scanf("%c", &queryType);
		getchar();

		printf("\nRealice la solicitud: ");
    	scanf("%s", query);
		getchar();

		printf("\nReporte\n%s\n", createMeeseeks(query));

		printf("\nDesea realizar otra solicitud? (S/N): ");
    	scanf("%c", &continueExecution);
		getchar();

		if(continueExecution == 'N' || continueExecution == 'n'){
			break;
		}
	}
	return 0;
}