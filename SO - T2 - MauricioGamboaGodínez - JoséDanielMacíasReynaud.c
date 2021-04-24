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
#include <time.h>
#include <errno.h>

//Meesesks's variables
static int *glob_var;
sem_t *bin_sem;

//char stack
char stack[25];
int top = -1;

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long tms) //https://qnaplus.com/c-program-to-sleep-in-milliseconds/
{
	struct timespec ts;
	int ret;

	if (tms < 0)
	{
		errno = EINVAL;
		return -1;
	}

	ts.tv_sec = tms / 1000;
	ts.tv_nsec = (tms % 1000) * 1000000;

	do
	{
		ret = nanosleep(&ts, &ts);
	} while (ret && errno == EINTR);

	return ret;
}

//Start of arithmetic operator
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

//End of arithmetic operator

int increment(float probability)
{
	int simulation = rand() % 100;

	if (probability >= simulation)
	{
		return 0;
	}

	int difference = simulation - probability;
	return difference;
}

//Generates the time in milliseconds of simulated execution time
int sleep_random(float prob)
{
	int miliseconds = 500;
	if (prob >= 0.0 && prob <= 45.0)
	{
		miliseconds = (rand() % (1500 - 500 + 1)) + 500;
	}
	else if (prob <= 85.0)
	{
		miliseconds = (rand() % (3000 - 1500 + 1)) + 1500;
	}
	else if (prob <= 100.0)
	{
		miliseconds = (rand() % (5000 - 3000 + 1)) + 3000;
	}
	return miliseconds;
}

//Generates the amount of children needed to solve the problem
int decide_child_amount(float prob)
{

	int amount = 1;
	if (prob >= 0.0 && prob <= 45.0)
	{
		amount = (rand() % (5 - 3 + 1)) + 3;
	}
	else if (prob <= 85.0)
	{
		amount = (rand() % (3 - 1 + 1)) + 1;
	}
	else if (prob <= 100.0)
	{
		amount = 1;
	}

	return amount;
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

char *send_to_box(char *request, char request_type, float probability, int level, int iteration)
{

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
		/*glob_var = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE,
						MAP_SHARED | MAP_ANONYMOUS, -1, 0);

		bin_sem = mmap(NULL, sizeof(bin_sem),
					   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
					   -1, 0);

		sem_init(bin_sem, 1, 1);
*/
		//*glob_var = 0;

		pid_t meeseeks;

		int resulttest = increment(probability);
		if (resulttest == 0 && level != 1)
		{
			printf("\nGoodbye!%d, PID: \n", getpid());

			exit(EXIT_SUCCESS);
		}
		int ref = 100 - resulttest;
		float finalresult = ref / 10.0f;

		probability += finalresult;

		int children_to_create = decide_child_amount(probability);
		printf("\n PROBABILIDAD: %f\n", probability);
		printf("\n QUIERO ESTA CANTIDAD DE HIJOS: %d\n", children_to_create);
		int var = level;
		int iteration = 1;
		for (int i = 0; i < children_to_create; i++)
		{

			meeseeks = create_meeseeks();
			if (meeseeks < 0)
			{
				exit(EXIT_FAILURE);
			}
			else if (meeseeks == 0)
			{
				var = level + 1;
				iteration = i;
				break;
			}
			else
			{
				//printf("Hi I'm Mr Meeseeks! I'm waiting for my children to finish (pid: %d, ppid: %d, N:%d)\n", getpid(), getppid(), level);

				waitpid(meeseeks, NULL, 0);
				//wait(NULL);
				//printf("Hi I'm Mr Meeseeks! My children finished(pid: %d, ppid: %d, N:%d)\n", getpid(), getppid(), level);
			}
		}

		if (meeseeks == 0)
		{
			int valor = 0;
			// proceso hijo
			//sem_wait(bin_sem);
			//int a = *glob_var;
			//a += 1;
			//a -= 1;
			//a += 1;
			//*glob_var = a;
			//a = *glob_var;
			//a -= 1;
			//a += 1;
			//a -= 1;
			//*glob_var = a;
			//a = *glob_var;
			//a += 1;
			//a -= 1;
			// return difference;
			// a += 1;
			//*glob_var = a;
			//sleep(1);
			//sem_getvalue(bin_sem, &valor);
			printf("Hi I'm Mr Meeseeks! Look at Meeeee. (pid: %d, ppid: %d, N:%d, i:%d)\n", getpid(), getppid(), level, iteration);
			msleep(sleep_random(probability));

			send_to_box(request, request_type, probability, var, iteration);

			//sem_post(bin_sem);
			//sem_getvalue(bin_sem, &valor);
			//printf("Hi I'm Mr Meeseeks! Look at Meeeee. (pid: %d, ppid: %d, N(:%d, i:%d)\n", getpid(), getppid(), level, valor);
			//Lentamente crece la facilidad hasta llegar a 100, cuando llega a 100, ya no hay hijos

			//int status = system(request);
			//kill(getpid(), SIGKILL);
		}
		else
		{
			// proceso padre

			//wait(NULL);
			//waitpid(meeseeks, NULL, 0);

			//CAMBIAR POR WAITPID
			//sleep(1);
			printf("Proceso Padre: pid:%d, ppid:%d \n", getpid(), getppid());
			printf("Hijo completado\n");
			//sem_destroy(bin_sem);
			//munmap(bin_sem, sizeof(bin_sem));
			//munmap(glob_var, sizeof *glob_var);
			//exit(EXIT_SUCCESS);
		}

		if (var != 1)
		{
			printf("\nVAR!%d\n", var);
			exit(EXIT_SUCCESS);
		}
		printf("soy otra cosa");
		return "Soy un P";

		break;
	default:;
		return "";
	}
}

int main(int argc, char **argv)
{
	srand(time(NULL) ^ getpid());
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

		printf("\nIngrese la dificultad (0.0 - 100.0): ");
		float prob_input;
		scanf("%f", &prob_input);

		//printf("%f es el numero", prob_input);
		//send_to_box(request, request_type, prob_input, 1);
		printf("\nReporte\n%s\n", send_to_box(request, request_type, prob_input, 1, 0));

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
