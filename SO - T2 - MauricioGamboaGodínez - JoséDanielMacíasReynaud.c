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

/*
Como compilar:
gcc "SO - T2 - MauricioGamboaGodínez - JoséDanielMacíasReynaud.c" -o "SO - T2 - MauricioGamboaGodínez - JoséDanielMacíasReynaud" -pthread

Ayuda con semaforos y pthread:
https://stackoverflow.com/questions/6847973/do-forked-child-processes-use-the-same-semaphore#:~:text=If%20you%20are%20using%20POSIX,with%20semaphores%20inside%20and%20forked
https://stackoverflow.com/questions/21129845/why-does-sem-open-work-with-fork-without-shared-memory



*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

static int *glob_var;

sem_t *bin_sem;

pid_t create_meeseeks()
{
	pid_t pid;

	pid = fork();
	return pid;
}

char *execute_query(char *query, char query_type)
{

	int res;

	switch (query_type)
	{
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
		glob_var = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE,
						MAP_SHARED | MAP_ANONYMOUS, -1, 0);

		bin_sem = mmap(NULL, sizeof(bin_sem),
					   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
					   -1, 0);

		sem_init(bin_sem, 1, 1);

		*glob_var = 0;

		pid_t meeseeks;
		for (int i = 0; i < 20; i++)
		{

			meeseeks = create_meeseeks();
			if (meeseeks < 0)
			{
				exit(EXIT_FAILURE);
			}
			else if (meeseeks == 0)
			{
				break;
			}
		}

		if (meeseeks < 0)
		{
			fprintf(stderr, "Mr.Meeseeks birth failed!");
			exit(EXIT_FAILURE);
		}
		else if (meeseeks == 0)
		{
			int valor;
			// proceso hijo
			sem_wait(bin_sem);
			int a = *glob_var;
			a += 1;
			a -= 1;
			a += 1;
			*glob_var = a;
			a = *glob_var;
			a -= 1;
			a += 1;
			a -= 1;
			*glob_var = a;
			a = *glob_var;
			a += 1;
			a -= 1;
			a += 1;
			*glob_var = a;
			sem_getvalue(bin_sem, &valor);
			printf("Mr.Meeseeks's child: pid:%d, ppid:%d, glob_var:%d, valor: %d\n", getpid(), getppid(), *glob_var, valor);
			sem_post(bin_sem);
			sem_getvalue(bin_sem, &valor);
			printf("Mr.Meeseeks's child: pid:%d, ppid:%d, glob_var:%d, valor: %d\n", getpid(), getppid(), *glob_var, valor);
			//int status = system(query);
			//kill(getpid(), SIGKILL);
			exit(EXIT_SUCCESS);
		}
		else
		{
			// proceso padre

			//wait(NULL);
			wait(NULL);
			sleep(1);
			printf("Proceso Padre: pid:%d, ppid:%d, glob_var:%d \n", getpid(), getppid(), *glob_var);
			printf("Hijo completado\n");
			sem_destroy(bin_sem);
			munmap(bin_sem, sizeof(bin_sem));
			munmap(glob_var, sizeof *glob_var);
		}

		return "Soy un P";

		break;
	default:;
		return "";
	}
}

int main(int argc, char **argv)
{
	char *query;
	char continue_exec;
	char query_type;

	while (1)
	{
		printf("\nConsulta textual (T)\n");
		printf("Operacion aritmetica (A)\n");
		printf("Operacion logica (L)\n");
		printf("Ejecutar programa (P)\n");
		printf("Indique el tipo de solicitud: ");
		scanf("%c", &query_type);
		getchar();

		printf("\nRealice la solicitud: ");
		scanf("%[^\n]%*c", query); //geeksforgeeks.org/taking-string-input-space-c-3-different-methods/

		printf("\nReporte\n%s\n", execute_query(query, query_type));
		printf("\nDesea realizar otra solicitud? (S/N): ");
		scanf("%c", &continue_exec);
		getchar();

		if (continue_exec == 'N' || continue_exec == 'n')
		{
			break;
		}
	}
	return 0;
}
