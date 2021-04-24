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
static int *isDone;
static int *count;

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
void send_to_box_simulation_aux(int children, int parent_to_child_request_old[], int parent_to_child_message_old[], int child_to_parent_solution_old[],size_t size)
{
    printf("Hi I'm Mr.Meeseeks, pid: %d,ppid: %d\n", getpid(), getppid());

    *count += 1;

    /*
        List with all possible bidereccional communications between the father and his children.
    */
    int pipes_parent_to_child_request[children][2];
    int pipes_parent_to_child_message[children][2];
    int pipes_child_to_parent_solution[children][2];

    pid_t pids[children];

    int data_parent = 0;
    int data_child;

    char buffer_message[BUFSIZ + 1];  // It'll contain a message from the parent
    char buffer_request[BUFSIZ + 1];  //It'll contain the request from the parent
    char buffer_solution[BUFSIZ + 1]; //It'll contain the solution if a process completes the request.

    close(parent_to_child_request_old[1]); //Close unused parent's writing
    close(parent_to_child_message_old[1]); 
    close(child_to_parent_solution_old[0]); //Closed unsed child's reading

    while (1)
    {
        data_parent = read(parent_to_child_request_old[0], buffer_request, size); //Read request from parent
        if(data_parent >0){
            break;
        }
    }
    close(parent_to_child_request_old[0]); //Closed parent's reading
    printf("Read %d bytes: %s pid: %d ppid: %d \n", data_parent, buffer_request, getpid(), getppid());
    printf("%d\n", *count);

    if (*count == 2)
    {
        *isDone = 1;
        char *solution = "LISTOdddddddddddddddd";
        close(parent_to_child_request_old[0]); //Closed parent's reading
        close(parent_to_child_message_old[0]);
        data_child = write(child_to_parent_solution_old[1], solution, strlen(solution));
        //printf("Wrote %d bytes pid %d\n", data_child, getpid());
        printf("Termine data: %d , pid: %d\n", data_child, getpid());
        printf("Goodbye, pid: %d,ppid: %d\n", getpid(), getppid());
        close(child_to_parent_solution_old[1]); //Closed child's writing
        exit(EXIT_SUCCESS);
    }

    int parent_to_child[2];
    int child_to_parent[2];

    if (pipe(parent_to_child) != 0 || pipe(child_to_parent) != 0)
    {
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < children; i++)
    {
        pid_t meeseeks;
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
            //send_to_box_secundary(children, parent_to_child, child_to_parent, strlen(buffer_request));
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
        data_parent = read(parent_to_child_message_old[0], buffer_message, 5);
        data_child = read(child_to_parent[0], buffer_solution, BUFSIZ);
        if (data_parent > 0 || data_child > 0)
        {
            break;
        }
    }
    
    close(parent_to_child_message_old[0]); //Closed parent's reading
    close(child_to_parent[0]);     //Closed child's reading

    if (data_parent > 0)
    {
        printf("Read %d bytes: %s PID %d\n", data_parent, buffer_message, getpid());
        //for (int i = 0; i < children; i++) //Send completation request to all children
        //{
            //data_parent = write(parent_to_child[1], buffer_request, strlen(buffer_request));
            //printf("Wrote %d bytes pid %d\n", data_parent, getpid());
        //}
    }
    else
    {
        printf("Read %d bytes: %s pid %d\n", data_child, buffer_solution, getpid());
        data_child = write(child_to_parent_solution_old[1], buffer_solution, strlen(buffer_solution));
        printf("Wrote %d bytes pid %d\n", data_child, getpid());
        for (int i = 0; i < children - 1; i++) //Process that send the completation request doesn't receive the message
        {
            char *message = "READY";
            data_child = write(parent_to_child[1], message, strlen(message));
            printf("Wrote %d bytes pid %d\n", data_child, getpid());
        }
    }
    close(child_to_parent_solution_old[1]);
    close(parent_to_child[1]); //Close parent's writing

    //waitMeeseeks(pids, children);
    printf("Goodbye, pid: %d,ppid: %d\n", getpid(), getppid());

    exit(EXIT_SUCCESS);
}

char *send_to_box_simulation(int children, char *request)
{

    printf("Hi I'm Mr.Meeseeks, pid: %d,ppid: %d\n", getpid(), getppid());

    *isDone = 0; //Zero represents that the request is not complete
    *count = 1;

    /*
        List with all possible bidereccional communications between the father and his children.
    */
    int pipes_parent_to_child_request[children][2];
    int pipes_parent_to_child_message[children][2];
    int pipes_child_to_parent_solution[children][2];

    pid_t pids[children];

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

        if (meeseeks < 0)
        {
            printf("%s\n", "Meeseeks Failed");
            exit(EXIT_FAILURE);
        }
        else if (meeseeks == 0)
        {
            send_to_box_simulation_aux(children, pipes_parent_to_child_request[child], pipes_parent_to_child_message[child], pipes_child_to_parent_solution[child], strlen(request));
        }
        else
        {
            pids[child] = meeseeks;
            data_child = write(pipes_parent_to_child_request[child][1], request, strlen(request));
            printf("Wrote %d bytes PID %d\n", data_child, getpid());
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

    for (int child = 0; child < children; child++)
    {
        close(pipes_parent_to_child_request[child][1]); //Close the parent's writing
        close(pipes_parent_to_child_message[child][1]); //Close the parent's writing
    }

    waitMeeseeks(pids, children);

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

    //Request
    char *solution = send_to_box_simulation(10, "Hola soy maci");
    printf("Solution: %s, isDone: %d, count: %d\n", solution, *isDone, *count);

    //Remove shared memory
    munmap(isDone, sizeof *isDone);
    munmap(count, sizeof *count);
    exit(EXIT_SUCCESS);
}