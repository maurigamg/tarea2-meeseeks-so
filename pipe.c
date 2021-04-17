#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

int main()
{
    int data_processed, data_processed2;
    int file_pipes[2];
    const char some_data[] = "123";
    char buffer[BUFSIZ + 1];
    pid_t fork_result;
    memset(buffer, '\0', sizeof(buffer));
    if (pipe(file_pipes) == 0)
    {
        fork_result = fork();
        if (fork_result == -1)
        {
            fprintf(stderr, "Fork failure");
            exit(EXIT_FAILURE);
        }
        if (fork_result == 0)
        {
            data_processed = read(file_pipes[0], buffer, BUFSIZ);
            printf("Read %d bytes: %s PID %d\n", data_processed, buffer, getpid());
            data_processed2 = write(file_pipes[1], "some_data", strlen(" "));
            printf("Wrote %d bytes PID %d\n", data_processed2, getpid());
            exit(EXIT_SUCCESS);
        }
        else
        {
            data_processed = write(file_pipes[1], some_data, strlen(some_data));
            printf("Wrote %d bytes PID %d\n", data_processed, getpid());
            data_processed2 = read(file_pipes[0], buffer, BUFSIZ);
            printf("Read %d bytes: %s PID %d\n", data_processed2, buffer, getpid());
        }
    }
    exit(EXIT_SUCCESS);
}