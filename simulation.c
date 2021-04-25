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

//
void send_to_box_secundary(int children, int parent_to_child_old[], int child_to_parent_old[], size_t size)
{
    printf("Hi I'm Mr.Meeseeks, pid: %d,ppid: %d\n", getpid(), getppid());
    pid_t meeseeks;

    int data_parent, data_child;
    *count += 1;

    int parent_to_child[2]; //Communication parent to child
    int child_to_parent[2]; //Communication child to parent

    char buffer_request[BUFSIZ + 1];  //It'll contain the request or another message from the parent
    char buffer_solution[BUFSIZ + 1]; //It'll contain the solution if a process completes it.

    close(parent_to_child_old[1]); //Close unused parent's writing
    close(child_to_parent_old[0]); //Closed unsed child's reading
    data_parent = read(parent_to_child_old[0], buffer_request, size); //Read request from parent
    printf("Read %d bytes: %s pid: %d ppid: %d \n", data_parent, buffer_request, getpid(), getppid());
    printf("%d\n", *count);

    if (*count == 3)
    {
        *isDone = 1;
        char *solution = "LISTOdddddddddddddddd";
        sem_wait(bin_sem);
        close(parent_to_child_old[0]); //Closed parent's reading
        data_child = write(child_to_parent_old[1], solution, strlen(solution));
        sem_post(bin_sem);
        //printf("Wrote %d bytes pid %d\n", data_child, getpid());
        printf("Termine data: %d , pid: %d\n", data_child, getpid());
        printf("Goodbye, pid: %d,ppid: %d\n", getpid(), getppid());
        close(child_to_parent_old[1]); //Closed child's writing
        exit(EXIT_SUCCESS);
    }

    if (pipe(parent_to_child) != 0 || pipe(child_to_parent) != 0)
    {
        exit(EXIT_FAILURE);
    }

    fcntl(child_to_parent[0], F_SETFL, O_NONBLOCK);
    fcntl(parent_to_child[0], F_SETFL, O_NONBLOCK);
    
    for (int i = 0; i < children; i++)
    {
        if (*isDone == 1)
        { //A process completed the request, is not necesary create more children
            break;
        }

        meeseeks = create_meeseeks();

        if (meeseeks < 0)
        {
            printf("%s\n", "Meeseeks Failed");
            exit(EXIT_FAILURE);
        }
        else if (meeseeks == 0)
        {
            send_to_box_secundary(children, parent_to_child, child_to_parent, strlen(buffer_request));
        }
        else
        {
            data_parent = write(parent_to_child[1], buffer_request, strlen(buffer_request));
            printf("Wrote %d bytes PID %d\n", data_parent, getpid());
            waitpid(meeseeks, NULL, 0);
        }
    }
    close(parent_to_child[0]); //Close unused parent's reading
    close(child_to_parent[1]); //Close unused child's writing
    while (1) //waiting for the completation message
    {
        data_parent = read(parent_to_child_old[0], buffer_request, 5);
        data_child = read(child_to_parent[0], buffer_solution, BUFSIZ);
        if (data_parent > 0 || data_child > 0)
        {
            break;
        }
    }
    close(parent_to_child_old[0]); //Closed parent's reading
    close(child_to_parent[0]);     //Closed child's reading

    if (data_parent > 0)
    {
        printf("Read %d bytes: %s PID %d\n", data_parent, buffer_request, getpid());
        for (int i = 0; i < children; i++) //Send completation request to all children
        {
            data_parent = write(parent_to_child[1], buffer_request, strlen(buffer_request));
            printf("Wrote %d bytes pid %d\n", data_parent, getpid());
        }
    }
    else
    {
        printf("Read %d bytes: %s pid %d\n", data_child, buffer_solution, getpid());
        data_child = write(child_to_parent_old[1], buffer_solution, strlen(buffer_solution));
        printf("Wrote %d bytes pid %d\n", data_child, getpid());
        for (int i = 0; i < children - 1; i++) //Process that send the completation request doesn't receive the message
        {
            char *message = "READY";
            data_child = write(parent_to_child[1], message, strlen(message));
            printf("Wrote %d bytes pid %d\n", data_child, getpid());
        }
    }
    close(child_to_parent_old[1]);
    close(parent_to_child[1]); //Close parent's writing

    printf("Goodbye, pid: %d,ppid: %d\n", getpid(), getppid());

    exit(EXIT_SUCCESS);
}

char *send_to_box_primary(int children, char *request)
{
    printf("Hi I'm Mr.Meeseeks, pid: %d,ppid: %d\n", getpid(), getppid());

    *isDone = 0; //Zero represents that the request is not complete
    *count = 1;

    /*
        List with all possible bidereccional communications between the father and his children.
    */
    int pipes_parent_to_child[children][2];
    int pipes_child_to_parent[children][2];
    pid_t pids[children];

    /*
        pipe's reading writing
    */
    int data_child; //It is used to know the result of the child's reading.

    char buffer_solution[BUFSIZ + 1]; //It'll contain the solution if a process completes it.

    //memset(buffer_request, '\0', sizeof(buffer_request));
    //memset(buffer_solution, '\0', sizeof(buffer_solution));

    for (int child = 0; child < children; child++)
    {
        /*
            meeseeks declaration
        */
        pid_t meeseeks;
        int parent_to_child[2]; //Communication parent to child
        int child_to_parent[2]; //Communication child to parent

        if (pipe(parent_to_child) != 0 || pipe(child_to_parent) != 0)
        {
            exit(EXIT_FAILURE);
        }
        pipes_parent_to_child[child][0] = parent_to_child[0];
        pipes_parent_to_child[child][1] = parent_to_child[1];
        pipes_child_to_parent[child][0] = child_to_parent[0];
        pipes_child_to_parent[child][1] = child_to_parent[1];
        fcntl(child_to_parent[0], F_SETFL, O_NONBLOCK);
        fcntl(parent_to_child[0], F_SETFL, O_NONBLOCK);

        meeseeks = create_meeseeks();

        if (meeseeks < 0)
        {
            printf("%s\n", "Meeseeks Failed");
            exit(EXIT_FAILURE);
        }
        else if (meeseeks == 0)
        {
            send_to_box_secundary(children, pipes_parent_to_child[child], pipes_child_to_parent[child], strlen(request));
        }
        else
        {
            pids[child] = meeseeks;
            data_child = write(pipes_parent_to_child[child][1], request, strlen(request));
            printf("Wrote %d bytes PID %d\n", data_child, getpid());
        }
    }

    for (int child = 0; child < children; child++)
    {
        close(pipes_parent_to_child[child][0]); //Close unused parent's reading
        close(pipes_child_to_parent[child][1]); //Close unused child's writing
    }

    data_child = 0;
    int index_child;
    while (data_child < 1)
    {
        for (int child = 0; child < children; child++)
        {
            
            data_child = read(pipes_child_to_parent[child][0], buffer_solution, BUFSIZ);
            printf("Read %d bytes: %s PID %d\n", data_child, buffer_solution, getpid());
            if (data_child > 0) //ANSWER??
            {
                index_child = child;
                break;
            }
        }
    }

    for (int child = 0; child < children; child++)
    {
        close(pipes_child_to_parent[child][0]); //Close the child's reading
    }

    for (int child = 0; child < children; child++) //Process that send the completation request doesn't receive the message
    {
        char *message = "READY";
        if (child != index_child)
        {
            data_child = write(pipes_parent_to_child[child][1], message, strlen(message));
        }
    }

    for (int child = 0; child < children; child++)
    {
        close(pipes_parent_to_child[child][1]); //Close the parent's writing
    }

    for (int meeseeks = 0; meeseeks < children; meeseeks++)
    {
        waitpid(pids[meeseeks], NULL, 0);
    }
    

    printf("Goodbye, pid: %d,ppid: %d\n", getpid(), getppid());

    char *solution = buffer_solution; //buffer_solution;
    return solution;
}

int main()
{
    /*
        Shared memory for all processes to access the variables
    */
    isDone = mmap(NULL, sizeof *isDone, PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    count = mmap(NULL, sizeof *count, PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    bin_sem = mmap(NULL, sizeof(bin_sem),
					   PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,
					   -1, 0);

	sem_init(bin_sem, 1, 1);

    //Request
    char *solution = send_to_box_primary(4, "Hola");
    printf("Solution: %s, isDone: %d, count: %d\n", solution, *isDone, *count);

    //Remove shared memory
    sem_destroy(bin_sem);
	munmap(bin_sem, sizeof(bin_sem));
    munmap(isDone, sizeof *isDone);
    munmap(count, sizeof *count);
    exit(EXIT_SUCCESS);
}