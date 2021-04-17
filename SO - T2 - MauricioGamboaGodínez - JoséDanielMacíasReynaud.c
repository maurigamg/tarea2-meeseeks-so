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
#include <string.h>
#include <ctype.h>

//char stack
char stack[25];
int top = -1;

int ipow(int base, int exp)
{
	int result = 1;
	for (;;)
	{
		if (exp & 1)
			result *= base;
		exp >>= 1;
		if (!exp)
			break;
		base *= base;
	}

	return result;
}

void push(char item)
{
	stack[++top] = item;
}

char pop()
{
	return stack[top--];
}

//returns precedence of operators
int precedence(char symbol)
{

	switch (symbol)
	{
	case '+':
	case '-':
		return 2;
		break;
	case '*':
	case '/':
		return 3;
		break;
	case '^':
		return 4;
		break;
	case '(':
	case ')':
	case '#':
		return 1;
		break;
	}
}

//check whether the symbol is operator?
int isOperator(char symbol)
{

	switch (symbol)
	{
	case '+':
	case '-':
	case '*':
	case '/':
	case '^':
	case '(':
	case ')':
		return 1;
		break;
	default:
		return 0;
	}
}

//converts infix expression to postfix
void convert(char infix[], char postfix[])
{
	int i, symbol, j = 0;
	stack[++top] = '#';

	for (i = 0; i < strlen(infix); i++)
	{
		symbol = infix[i];

		if (isOperator(symbol) == 0)
		{
			postfix[j] = symbol;
			j++;
		}
		else
		{
			if (symbol == '(')
			{
				push(symbol);
			}
			else
			{
				if (symbol == ')')
				{

					while (stack[top] != '(')
					{
						postfix[j] = pop();
						j++;
					}

					pop(); //pop out (.
				}
				else
				{
					if (precedence(symbol) > precedence(stack[top]))
					{
						push(symbol);
					}
					else
					{

						while (precedence(symbol) <= precedence(stack[top]))
						{
							postfix[j] = pop();
							j++;
						}

						push(symbol);
					}
				}
			}
		}
	}

	while (stack[top] != '#')
	{
		postfix[j] = pop();
		j++;
	}

	postfix[j] = '\0'; //null terminate string.
}

//int stack
int stack_int[25];
int top_int = -1;

void push_int(int item)
{
	stack_int[++top_int] = item;
}

char pop_int()
{
	return stack_int[top_int--];
}

//evaluates postfix expression
int evaluate(char *postfix)
{

	char ch;
	int i = 0, operand1, operand2;

	while ((ch = postfix[i++]) != '\0')
	{

		if (isdigit(ch))
		{
			push_int(ch - '0'); // Push the operand
		}
		else
		{
			//Operator,pop two  operands
			operand2 = pop_int();
			operand1 = pop_int();

			switch (ch)
			{
			case '+':
				push_int(operand1 + operand2);
				break;
			case '-':
				push_int(operand1 - operand2);
				break;
			case '*':
				push_int(operand1 * operand2);
				break;
			case '/':
				push_int(operand1 / operand2);
				break;
			case '^':
				push_int(ipow(operand1, operand2));
				break;
			}
		}
	}

	return stack_int[top_int];
}

void demo_evaluate()
{
	char infix[25] = "2^(3-1)", postfix[25];
	convert(infix, postfix);

	printf("Infix expression is: %s\n", infix);
	printf("Postfix expression is: %s\n", postfix);
	printf("Evaluated expression is: %d\n", evaluate(postfix));
}

pid_t create_meeseeks()
{
	pid_t pid;

	pid = fork();
	return pid;
}

int execute_program_request()
{
}

char *send_to_box(char *request, char request_type)
{

	static int *glob_var;

	sem_t *bin_sem;

	switch (request_type)
	{
	case 'T':;
		return "Soy un texto";
		break;
	case 'A':;
		return "Soy un aritmetico";
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
			printf("Hi I'm Mr Meeseeks! Look at Meeeee. (pid: %d, ppid: %d, N(no hecho):%d, i:%d)\n", getpid(), getppid(), *glob_var, valor);
			sem_post(bin_sem);
			sem_getvalue(bin_sem, &valor);
			printf("Hi I'm Mr Meeseeks! Look at Meeeee. (pid: %d, ppid: %d, N(no hecho):%d, i:%d)\n", getpid(), getppid(), *glob_var, valor);
			//int status = system(request);
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
	char *request;
	char continue_exec;
	char request_type;
	int tareas = 0;

	while (1)
	{
		printf("\nConsulta textual (T)\n");
		printf("Operacion aritmetica (A)\n");
		printf("Operacion logica (L)\n");
		printf("Ejecutar programa (P)\n");
		printf("Indique el tipo de solicitud: ");
		scanf("%c", &request_type);
		getchar();

		printf("\nRealice la solicitud: ");
		scanf("%[^\n]%*c", request); //geeksforgeeks.org/taking-string-input-space-c-3-different-methods/

		printf("\nReporte\n%s\n", send_to_box(request, request_type));
		tareas += 1;
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
