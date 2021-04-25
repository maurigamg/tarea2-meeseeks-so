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

/*
    Global variable to know if a process resolved the request,
    this prevents other processes from solving the problem when the
    completion message is passing between all created processes.
    */
static int *isDone;
static int *count;
sem_t *bin_sem;

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
void send_to_box_simulation_aux(int children, int parent_to_child_request_old[], int parent_to_child_message_old[], int child_to_parent_solution_old[])
{
    printf("Hi I'm Mr.Meeseeks, pid: %d,ppid: %d\n", getpid(), getppid());
    sem_wait(bin_sem);
    *count += 1;
    sem_post(bin_sem);

    int data_parent = 0;
    int data_child = 0;

    close(parent_to_child_request_old[1]); //Close unused parent's writing
    close(parent_to_child_message_old[1]); 
    close(child_to_parent_solution_old[0]); //Closed unsed child's reading
    sem_wait(bin_sem);
    if (*count == 10)
    {
        *isDone = 1;
        sem_post(bin_sem);
        char buffer_solution[100];
        sprintf(buffer_solution, "Process with pid %d and ppid %d completed it", getpid(), getppid());
        close(parent_to_child_request_old[0]); //Closed parent's reading
        close(parent_to_child_message_old[0]);
        data_child = write(child_to_parent_solution_old[1], buffer_solution, strlen(buffer_solution));
        //printf("Wrote %d bytes pid %d\n", data_child, getpid());
        printf("I completed it!! - pid %d - ppid: %d\n", getpid(), getppid());
        printf("Goodbye, pid: %d,ppid: %d\n", getpid(), getppid());
        close(child_to_parent_solution_old[1]); //Closed child's writing
        exit(EXIT_SUCCESS);
    }
    sem_post(bin_sem);
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
    printf("Read %d bytes: %s pid: %d ppid: %d \n", data_parent, buffer_request, getpid(), getppid());
    printf("%d\n", *count);

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
            send_to_box_simulation_aux(children, pipes_parent_to_child_request[child], pipes_parent_to_child_message[child], pipes_child_to_parent_solution[child]);
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
        printf("Read %d bytes: %s PID %d\n", data_parent, buffer_message, getpid());
        for (int child = 0; child < children_created; child++) //Send completation request to all children
        {
            char * message = buffer_message;
            data_parent = write(pipes_parent_to_child_message[child][1], message, strlen(message));
        }
    }
    else
    {
        printf("Read %d bytes: %s pid %d\n", data_child, buffer_solution, getpid());
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

    printf("Goodbye, pid: %d,ppid: %d\n", getpid(), getppid());

    exit(EXIT_SUCCESS);
}

char *send_to_box_simulation(int children, char *request)
{

    printf("Hi I'm Mr.Meeseeks, pid: %d,ppid: %d\n", getpid(), getppid());

    *isDone = 0; //Zero represents that the request is not complete
    *count = 1;

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
            send_to_box_simulation_aux(children, pipes_parent_to_child_request[child], pipes_parent_to_child_message[child], pipes_child_to_parent_solution[child]);
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
    printf("Read: solution %s PID %d\n", buffer_solution, getpid());
    printf("Goodbye, pid: %d,ppid: %d\n", getpid(), getppid());

    char *solution = buffer_solution; //buffer_solution;
    return solution;
}

int main()
{
    bin_sem = mmap(NULL, sizeof(bin_sem),
					   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
					   -1, 0);

	sem_init(bin_sem, 1, 1);
    /*
        Shared memory for all processes to access the variables
    */
    isDone = mmap(NULL, sizeof *isDone, PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    count = mmap(NULL, sizeof *count, PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //Request
    char *solution = send_to_box_simulation(2, "Hola soy maci");
    printf("Solution: %s, isDone: %d, count: %d\n", solution, *isDone, *count);

    //Remove shared memory
    sem_destroy(bin_sem);
	munmap(bin_sem, sizeof(bin_sem));
    munmap(isDone, sizeof *isDone);
    munmap(count, sizeof *count);
    exit(EXIT_SUCCESS);
}