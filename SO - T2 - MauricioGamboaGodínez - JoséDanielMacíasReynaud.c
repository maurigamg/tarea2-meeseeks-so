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
#include <signal.h>

pid_t create_meeseeks(){
	pid_t pid;

	pid = fork();
	return pid;
}

char * execute_query(char *query, char query_type){
	switch (query_type){
		case 'T':;
			return "Soy un texto";
			break;
		case 'A':;
			return "Soy un algoritmo";
			break;
		case 'L':;
			return "Soy un logico";
			break;
		case 'P':;
			pid_t meeseeks = create_meeseeks();
			if (meeseeks < 0){
				fprintf(stderr, "Mr.Meeseeks birth failed!");
			}else if (meeseeks == 0){
				// proceso hijo
				printf("Mr.Meeseeks's child: pid:%d, ppid:%d! \n", getpid(), getppid());
				int status = system(query);
				printf("I'm done!!\n");
				kill(getpid(), SIGKILL);
			}else{
				// proceso padre
				printf("Proceso Padre: pid:%d, ppid:%d! \n", getpid(), getppid());
				waitpid(meeseeks, NULL, 0);
				printf("Hijo completado\n");
			}

			return "Soy un P";

			break;
		default:;
			return "";
	}
}

int main(int argc, char **argv){
	char *query;
	char continue_exec = 'S';
	char query_type = 'S';

	int a = atoi("2 + 2");
	printf("%d", a);
	// while (1){
	// 	printf("\nConsulta textual (T)\n");
	// 	printf("Operacion aritmetica (A)\n");
	// 	printf("Operacion logica (L)\n");
	// 	printf("Ejecutar programa (P)\n");
	// 	printf("Indique el tipo de solicitud: ");
	// 	scanf("%c", &query_type);
	// 	getchar();

	// 	printf("\nRealice la solicitud: ");
	// 	scanf("%[^\n]%*c", query); //geeksforgeeks.org/taking-string-input-space-c-3-different-methods/

	// 	printf("\nReporte\n%s\n", execute_query(query, query_type));

	// 	printf("\nDesea realizar otra solicitud? (S/N): ");
	// 	scanf("%c", &continue_exec);
	// 	getchar();

	// 	if (continue_exec == 'N' || continue_exec == 'n'){
	// 		break;
	// 	}
	// }
	return 0;
}