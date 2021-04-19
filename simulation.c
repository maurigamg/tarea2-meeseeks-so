#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

int main()
{
    int data_processed, data_processed2;

    int file_pipes[2];
    int file_pipes_2[2];

    const char * message = "Sabe";
    const char * message2 = "Hola";

    char buffer_request[BUFSIZ + 1];
    char buffer_solution[BUFSIZ + 1];

    pid_t fork_result;

    memset(buffer_request, '\0', sizeof(buffer_request));
    memset(buffer_solution, '\0', sizeof(buffer_solution));

    if (pipe(file_pipes) == 0 && pipe(file_pipes_2) == 0)
    {
        int i = 0;
        while(i < 2)
        {
            fork_result = fork();

            if(fork_result == 0){
                break;
            }
            i++;
            printf("%s\n", "Hola mau");
        }
        
        if (fork_result == -1)
        {
            fprintf(stderr, "Fork failure");
            exit(EXIT_FAILURE);
        }
        if (fork_result == 0)
        {
            printf("%s\n", "Hola maci");
            data_processed = read(file_pipes[0], buffer_request, BUFSIZ);
            close(file_pipes[0]);
            printf("Read %d bytes: %s PID %d\n", data_processed, buffer_request, getpid());
            data_processed2 = write(file_pipes_2[1], message2, strlen(message2));
            printf("Wrote %d bytes PID %d\n", data_processed2, getpid());
            close(file_pipes_2[1]);
            close(file_pipes[1]);
            close(file_pipes_2[0]);
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(file_pipes[0]);
            close(file_pipes_2[1]);
            data_processed = write(file_pipes[1], message, strlen(message));
            close(file_pipes[1]);
            printf("Wrote %d bytes PID %d\n", data_processed, getpid());
            // while(1){
                data_processed2 = read(file_pipes_2[0], buffer_solution, BUFSIZ);
                close(file_pipes_2[0]);
            //     if(data_processed2 < 1){
            //         break;
            //     }
                 printf("Read %d bytes: %s PID %d\n", data_processed2, buffer_solution, getpid());
            // }      
        }
    }
    exit(EXIT_SUCCESS);
}