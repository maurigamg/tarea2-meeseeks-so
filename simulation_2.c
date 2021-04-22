#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

/*
    Global variable to know if a process resolved the request,
    this prevents other processes from solving the problem when the
    completion message is passing between all created processes.
    */
static int *realizado;

pid_t create_meeseeks()
{
    pid_t pid;
    pid = fork();
    return pid;
}

void send_to_box_secundary(int children, int parent_to_child_old[], int child_to_parent_old[], size_t size)
{
    int data_processed;

    *realizado += 1;

    char buffer_request[BUFSIZ + 1];  //It'll contain the request to send to the children
    char buffer_solution[BUFSIZ + 1]; //It'll contain the solution if a process completes it.

    close(parent_to_child_old[1]); //Close unused write for parent
    close(child_to_parent_old[0]); //Closed unsed read for child
    data_processed = read(parent_to_child_old[0], buffer_request, size);
    printf("Read %d bytes: %s pid: %d ppid: %d \n", data_processed, buffer_request, getpid(), getppid());
    close(parent_to_child_old[0]);
    int i = 0;
    while (i < 5) //Cuidado
    {
        data_processed = write(child_to_parent_old[1], "Llega", strlen("Llega"));
        printf("Wrote %d bytes pid: %d ppid: %d \n", data_processed, getpid(), getppid());
        i++;
    }
    close(child_to_parent_old[1]);
    exit(EXIT_SUCCESS);
}

char *send_to_box_primary(int children, char *request)
{
    /*
        meeseeks declaration
    */
    pid_t meeseeks;

    *realizado = 0; //Zero represents that the request is not complete

    /*
        Pipe initialization
    */
    int data_processed, data_processed2; //Used to know of a result from read and write

    int parent_to_child[2]; //Communication parent to child
    int child_to_parent[2]; //Communication child to parent

    char buffer_request[BUFSIZ + 1];  //It'll contain the request to send to the children
    char buffer_solution[BUFSIZ + 1]; //It'll contain the solution if a process completes it.

    //memset(buffer_request, '\0', sizeof(buffer_request));
    //memset(buffer_solution, '\0', sizeof(buffer_solution));
    if (pipe(parent_to_child) != 0 || pipe(child_to_parent) != 0)
    {
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < children; i++)
    {
        meeseeks = create_meeseeks();
        if (meeseeks < 0)
        {
            printf("%s\n", "Meeseeks Failed");
            exit(EXIT_FAILURE);
        }
        else if (meeseeks == 0)
        {
            send_to_box_secundary(children - 1, parent_to_child, child_to_parent, strlen(request));
            //close(parent_to_child[0]);
            //close(child_to_parent[1]);
        }
        else
        {
            data_processed = write(parent_to_child[1], request, strlen(request));
            printf("Wrote %d bytes PID %d\n", data_processed, getpid());
            waitpid(meeseeks, NULL, 0);
        }
    }
    close(parent_to_child[0]); //Close unused read for parent
    close(child_to_parent[1]); //Close unused write for child
    close(parent_to_child[1]);
    int i = 0;
    while (i < 25)
    {
        data_processed2 = read(child_to_parent[0], buffer_solution, 5);
        printf("Read %d bytes: %s PID %d\n", data_processed2, buffer_solution, getpid());
        i++;
    }
    close(child_to_parent[0]);

    char *solution = ""; //buffer_solution;
    return solution;
}

int main()
{
    /*
        Shared memory for all processes to access the variable
    */
    realizado = mmap(NULL, sizeof *realizado, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    send_to_box_primary(4, "Hola");
    printf("Realizados: %d\n", *realizado);
    munmap(realizado, sizeof *realizado);
    /*
    int data_processed, data_processed2;
    int parent_to_child[2];
    int child_to_parent[2];
    const char *message = "Sabe";
    const char *message2 = "Hola";
    char buffer_request[BUFSIZ + 1];
    char buffer_solution[BUFSIZ + 1];
    pid_t fork_result;
    //memset(buffer_request, '\0', sizeof(buffer_request));
    //memset(buffer_solution, '\0', sizeof(buffer_solution));
    if (pipe(parent_to_child) == 0 && pipe(child_to_parent) == 0)
    {
        fork_result = fork();
        if (fork_result == -1)
        {
            fprintf(stderr, "Fork failure");
            exit(EXIT_FAILURE);
        }
        if (fork_result == 0)
        {
            close(parent_to_child[1]);
            close(child_to_parent[0]);
            data_processed = read(parent_to_child[0], buffer_request, BUFSIZ);
            close(parent_to_child[0]);
            printf("Read %d bytes: %s PID %d\n", data_processed, buffer_request, getpid());
            int i = 0;
            while (i<10)
            {
                data_processed2 = write(child_to_parent[1], message2, strlen(message2));
                printf("Wrote %d bytes PID %d\n", data_processed2, getpid());
                i++;
            }
            close(child_to_parent[1]);
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(parent_to_child[0]);
            close(child_to_parent[1]);
            data_processed = write(parent_to_child[1], message, strlen(message));
            close(parent_to_child[1]);
            printf("Wrote %d bytes PID %d\n", data_processed, getpid());
            int i = 0;
            while (i<14)
            {
                data_processed2 = read(child_to_parent[0], buffer_solution, 4);
                printf("Read %d bytes: %s PID %d\n", data_processed2, buffer_solution, getpid());
                i++;
            }
            close(child_to_parent[0]);
        }
    }*/
    exit(EXIT_SUCCESS);
}