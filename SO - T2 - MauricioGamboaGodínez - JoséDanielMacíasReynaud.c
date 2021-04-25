/*
ITCR, Sede Cartago
Escuela de Computación, IC-6600 Principios de Sistemas Operativos
SO - Tarea 2 - Mr. Meeseeks
Estudiantes:
	Mauricio Gamboa Godínez 2018113173
	José Daniel Macías Reynaud 2018241572
Profesor: Esteban Arias Méndez
Fecha de entrega: 24 de abril, 2021, I Semestre
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
#include <fcntl.h> 
#include <time.h>
#include <errno.h>

/*
    Global variable to know if a process resolved the request,
    this prevents other processes from solving the problem when the
    completion message is passing between all created processes.
    */
static int *isDone;
static int *count;
sem_t *bin_sem;
sem_t *bin_sem_2;
static int *iterations;

//char stack
char stack[25];
int top = -1;

int time_to_go = 0;
int time_is_over = 0;
clock_t initial_time;

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
	if (probability == 0)
	{
		return 0;
	}

	int simulation = rand() % 100;

	if (probability >= simulation)
	{
		return 0;
	}

	int difference = simulation - probability;
	return difference;
}

void set_global_timer_minutes(float minutes)
{
	int ms = minutes * 60 * 1000;
	time_to_go = ms;
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

void waitMeeseeks(pid_t pids[], int children)
{
    for (int meeseeks = 0; meeseeks < children; meeseeks++)
    {
        waitpid(pids[meeseeks], NULL, 0);
    }
}

//
void send_to_box_simulation_aux(int level, int instancia, float probability, int parent_to_child_request_old[], int parent_to_child_message_old[], int child_to_parent_solution_old[])
{
    printf("Hi I'm Mr Meeseeks! Look at Meeeee. (pid: %d, ppid: %d, N: %d, i: %d)\n", getpid(), getppid(), level, instancia);
    sem_wait(bin_sem);
    *count += 1;
    sem_post(bin_sem);

    int children = decide_child_amount(probability);

    int data_parent = 0;
    int data_child = 0;

    close(parent_to_child_request_old[1]); //Close unused parent's writing
    close(parent_to_child_message_old[1]); 
    close(child_to_parent_solution_old[0]); //Closed unsed child's reading

    int resulttest = increment(probability);

    sem_wait(bin_sem);
    if (resulttest == 0 && *isDone == 0)
    {
        *isDone = 1;
        sem_post(bin_sem);
        char buffer_solution[100];
        sprintf(buffer_solution, "Process with (pid: %d, ppid: %d, N: %d, i: %d) completed it", getpid(), getppid(), level, instancia);
        close(parent_to_child_request_old[0]); //Closed parent's reading
        close(parent_to_child_message_old[0]);
        data_child = write(child_to_parent_solution_old[1], buffer_solution, strlen(buffer_solution));
        //printf("Wrote %d bytes pid %d\n", data_child, getpid());
        printf("I completed it!! (pid: %d, ppid: %d, N: %d, i: %d)\n", getpid(), getppid(), level, instancia);
        printf("Goodbye!! (pid: %d, ppid: %d, N: %d, i: %d)\n", getpid(), getppid(), level, instancia);
        close(child_to_parent_solution_old[1]); //Closed child's writing
        exit(EXIT_SUCCESS);
    }
    sem_post(bin_sem);

    if(probability != 0){
        int ref = 100 - resulttest;
	    float finalresult = ref / 10.0f;
        probability += finalresult;;
    }

    /*
        List with all possible bidereccional communications between the father and his children.
    */
    int pipes_parent_to_child_request[children][2];
    int pipes_parent_to_child_message[children][2];
    int pipes_child_to_parent_solution[children][2];

    pid_t pids[children];

    char buffer_message[BUFSIZ + 1];  // It'll contain a message from the parent
    char buffer_request[BUFSIZ + 1];  //It'll contain the request from the parent
    char buffer_solution[BUFSIZ + 1]; //It'll contain the solution if a process completes the request.

    data_parent = read(parent_to_child_request_old[0], buffer_request, BUFSIZ); //Read request from parent

    close(parent_to_child_request_old[0]); //Closed parent's reading
    printf("Read %d bytes: %s (pid: %d, ppid: %d, N: %d, i: %d)\n", data_parent, buffer_request, getpid(), getppid(), level, instancia);

    int children_created = 0;
    char * request = buffer_request;
    for (int child = 0; child < children; child++)
    {
        if (*isDone == 1)
        { //A process completed the request, is not necesary create more children
            break;
        }
        
        /*
            meeseeks declaration
        */
        pid_t meeseeks;
        int parent_to_child_request[2];  //Communication parent to child for request
        int parent_to_child_message[2];  //Communication parent to child for completation message
        int child_to_parent_solution[2]; //Communication child to parent for solution

        if (pipe(parent_to_child_request) != 0 || pipe(parent_to_child_message) != 0 || pipe(child_to_parent_solution) != 0)
        {
            exit(EXIT_FAILURE);
        }
        pipes_parent_to_child_request[child][0] = parent_to_child_request[0];
        pipes_parent_to_child_request[child][1] = parent_to_child_request[1];
        pipes_parent_to_child_message[child][0] = parent_to_child_message[0];
        pipes_parent_to_child_message[child][1] = parent_to_child_message[1];
        pipes_child_to_parent_solution[child][0] = child_to_parent_solution[0];
        pipes_child_to_parent_solution[child][1] = child_to_parent_solution[1];

        fcntl(parent_to_child_message[0], F_SETFL, O_NONBLOCK);
        fcntl(child_to_parent_solution[0], F_SETFL, O_NONBLOCK);

        meeseeks = create_meeseeks();

        if (meeseeks < 0)
        {
            exit(EXIT_FAILURE);
        }
        else if (meeseeks == 0)
        {
            sem_wait(bin_sem_2);
            iterations[level] = iterations[level] + 1;
            int new_instancia = iterations[level];
            sem_post(bin_sem_2);
            send_to_box_simulation_aux(level + 1, new_instancia, probability, pipes_parent_to_child_request[child], pipes_parent_to_child_message[child], pipes_child_to_parent_solution[child]);
        }
        else
        {
            pids[child] = meeseeks;
            char * request = buffer_request;
            data_child = write(pipes_parent_to_child_request[child][1], request, strlen(request));
        }
        children_created += 1;
    }

    for (int child = 0; child < children_created; child++)
    {
        close(pipes_parent_to_child_request[child][0]); //Close unused parent's reading
        close(pipes_parent_to_child_message[child][0]); //Close unused parent's reading
        close(pipes_child_to_parent_solution[child][1]); //Close unused child's writing
    }

    data_child = 0;
    data_parent = 0;
    int index_child;

    while (data_child < 1 && data_parent < 1)
    {
        data_parent = read(parent_to_child_message_old[0], buffer_message, 5);
        for (int child = 0; child < children_created; child++)
        {
            data_child = read(pipes_child_to_parent_solution[child][0], buffer_solution, BUFSIZ);
            if (data_child > 0) //ANSWER??
            {
                index_child = child;
                break;
            }
        }
    }

    close(parent_to_child_message_old[0]); //Closed parent's reading

    for (int child = 0; child < children_created; child++)
    {
        close(pipes_child_to_parent_solution[child][0]); //Close the child's reading
    }

    if (data_parent > 0)
    {
        printf("Read Message: %d bytes: %s (pid: %d, ppid: %d, N: %d, i: %d)n", data_parent, buffer_message, getpid(), getppid(), level, instancia);
        for (int child = 0; child < children_created; child++) //Send completation request to all children
        {
            char * message = buffer_message;
            data_parent = write(pipes_parent_to_child_message[child][1], message, strlen(message));
        }
    }
    else
    {
        printf("Read Solution: %d bytes: %s (pid: %d, ppid: %d, N: %d, i: %d)\n", data_child, buffer_solution, getpid(), getppid(), level, instancia);
        data_child = write(child_to_parent_solution_old[1], buffer_solution, strlen(buffer_solution));
        char *message = "READY";
        for (int child = 0; child < children_created ; child++) //Process that send the completation request doesn't receive the message
        {
            if (child != index_child)
            {
                data_child = write(pipes_parent_to_child_message[child][1], message, strlen(message));
            }
        }
    }

    waitMeeseeks(pids, children_created);

    close(child_to_parent_solution_old[1]);

    for (int child = 0; child < children_created; child++)
    {
        close(pipes_parent_to_child_request[child][1]); //Close the parent's writing
        close(pipes_parent_to_child_message[child][1]); //Close the parent's writing
    }

    printf("Goodbye!! (pid: %d, ppid: %d, N: %d, i: %d)\n", getpid(), getppid(), level, instancia);

    exit(EXIT_SUCCESS);
}

char *send_to_box_simulation(char *request, float probability)
{

    printf("Hi I'm Mr Meeseeks! Look at Meeeee. (pid: %d, ppid: %d, N: 1, i: 1)\n", getpid(), getppid());

    *isDone = 0; //Zero represents that the request is not complete
    *count = 1;
    int level = 1;
    int children = decide_child_amount(probability);

    pid_t pids[children];

    /*
        List with all possible bidereccional communications between the father and his children.
    */
    int pipes_parent_to_child_request[children][2];
    int pipes_parent_to_child_message[children][2];
    int pipes_child_to_parent_solution[children][2];

    /*
        Pipe initialization
    */
    int data_child; //It is used to know the result of the child's reading.

    char buffer_solution[BUFSIZ + 1]; //It'll contain the solution if a process completes it.
    
    for (int child = 0; child < children; child++)
    {
        /*
            meeseeks declaration
        */
        pid_t meeseeks;
        int parent_to_child_request[2];  //Communication parent to child for request
        int parent_to_child_message[2];  //Communication parent to child for completation message
        int child_to_parent_solution[2]; //Communication child to parent for solution

        if (pipe(parent_to_child_request) != 0 || pipe(parent_to_child_message) != 0 || pipe(child_to_parent_solution) != 0)
        {
            exit(EXIT_FAILURE);
        }
        pipes_parent_to_child_request[child][0] = parent_to_child_request[0];
        pipes_parent_to_child_request[child][1] = parent_to_child_request[1];
        pipes_parent_to_child_message[child][0] = parent_to_child_message[0];
        pipes_parent_to_child_message[child][1] = parent_to_child_message[1];
        pipes_child_to_parent_solution[child][0] = child_to_parent_solution[0];
        pipes_child_to_parent_solution[child][1] = child_to_parent_solution[1];

        meeseeks = create_meeseeks();
        fcntl(parent_to_child_message[0], F_SETFL, O_NONBLOCK);
        fcntl(child_to_parent_solution[0], F_SETFL, O_NONBLOCK);

        if (meeseeks < 0)
        {
            printf("%s\n", "Meeseeks Failed");
            exit(EXIT_FAILURE);
        }
        else if (meeseeks == 0)
        {
            sem_wait(bin_sem_2);
            iterations[level] = iterations[level] + 1;
            int instancia = iterations[level];
            sem_post(bin_sem_2);
            send_to_box_simulation_aux(level + 1, instancia, probability, pipes_parent_to_child_request[child], pipes_parent_to_child_message[child], pipes_child_to_parent_solution[child]);
        }
        else
        {
            pids[child] = meeseeks;
            data_child = write(pipes_parent_to_child_request[child][1], request, strlen(request));
        }
    }

    for (int child = 0; child < children; child++)
    {
        close(pipes_parent_to_child_request[child][0]); //Close unused parent's reading
        close(pipes_parent_to_child_message[child][0]); //Close unused parent's reading
        close(pipes_child_to_parent_solution[child][1]); //Close unused child's writing
    }

    data_child = 0;
    int index_child;
    while (data_child < 1)
    {
        for (int child = 0; child < children; child++)
        {
            data_child = read(pipes_child_to_parent_solution[child][0], buffer_solution, BUFSIZ);
            if (data_child > 0) //ANSWER??
            {
                index_child = child;
                break;
            }
        }
    }
    for (int child = 0; child < children; child++)
    {
        close(pipes_child_to_parent_solution[child][0]); //Close the child's reading
    }

    for (int child = 0; child < children; child++) //Process that send the completation request doesn't receive the message
    {
        char *message = "READY";
        if (child != index_child)
        {
            data_child = write(pipes_parent_to_child_message[child][1], message, strlen(message));
        }
    }

    waitMeeseeks(pids, children);

    for (int child = 0; child < children; child++)
    {
        close(pipes_parent_to_child_request[child][1]); //Close the parent's writing
        close(pipes_parent_to_child_message[child][1]); //Close the parent's writing
    }
    printf("Read Solution: %s (pid: %d, ppid: %d, N: 1, i: 1)\n", buffer_solution, getpid(), getppid());
    printf("Goodbye!! (pid: %d, ppid: %d, N: 1, i: 1)\n", getpid(), getppid());

    char *solution = buffer_solution; //buffer_solution;
    return solution;
}

char *send_to_box(char *request, char request_type, float probability, int minutos)
{

	switch (request_type)
	{
	case 'T':;
        char * rs = send_to_box_simulation(request, probability);
        wait(NULL);
		return rs;
        break;
	case 'A':;
		return "Soy un aritmetico";
		break;
	case 'L':;
		return "Soy un logico";
		break;
	case 'P':;
			//int status = system(request);
			//kill(getpid(), SIGKILL);
	default:;
		return "";
	}
}

int main()
{
    srand(time(NULL) ^ getpid());
	char *request;
	char continue_exec = 'S';
	char request_type;
	int tareas = 0;
    float prob_input;
    float minutos;

    iterations = mmap(NULL, 25 * sizeof(int),
					  PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED,
					  -1, 0);


    bin_sem = mmap(NULL, sizeof(bin_sem),
					   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
					   -1, 0);
    bin_sem_2 = mmap(NULL, sizeof(bin_sem_2),
					   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
					   -1, 0);

	sem_init(bin_sem, 1, 1);
    sem_init(bin_sem_2, 1, 1);
    /*
        Shared memory for all processes to access the variables
    */
    isDone = mmap(NULL, sizeof *isDone, PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    count = mmap(NULL, sizeof *count, PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //Request
    //char *solution = send_to_box_simulation("Hola soy maci", 15);
    //printf("Solution: %s, isDone: %d, count: %d\n", solution, *isDone, *count);
    while (1)
	{
		printf("\nConsulta textual (T)\n");
		printf("Operacion aritmetica (A)\n");
		printf("Operacion logica (L)\n");
		printf("Ejecutar programa (P)\n");
		printf("Indique el tipo de solicitud: ");
		scanf("%c", &request_type);
		getchar();

		//printf("\nRealice la solicitud: ");
		//scanf("%[^\n]%*c", request); //geeksforgeeks.org/taking-string-input-space-c-3-different-methods/
        //printf("%s\n",request);
        if(request_type == 'T'){
            printf("\nIngrese la dificultad (0.0 - 100.0): ");
		    scanf("%f", &prob_input);

		    printf("\nIngrese los minutos máximos: ");
		    scanf("%f", &minutos);
        }

        printf("\nReporte\n%s\n", send_to_box(request, request_type, prob_input, minutos));

		printf("\nDesea realizar otra solicitud? (S/N): ");
		scanf("%c", &continue_exec);
		getchar();

		if (continue_exec == 'N' || continue_exec == 'n')
		{
			break;
		}
        tareas += 1;
	}
    char buffer_report[100];
    sprintf(buffer_report, "\nTareas realizadas: %d\n Instancias creadas: %d\n", tareas, *count);
    char * report = buffer_report;
    printf("%s",report);

    //Remove share"d memory
    sem_destroy(bin_sem);
	munmap(bin_sem, sizeof(bin_sem));
    munmap(bin_sem_2, sizeof(bin_sem_2));
    munmap(isDone, sizeof *isDone);
    munmap(count, sizeof *count);
    munmap(iterations, sizeof *iterations);
    exit(EXIT_SUCCESS);
}