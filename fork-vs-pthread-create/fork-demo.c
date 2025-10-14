#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("Program started. PID: %d\n", getpid());

    int pid = fork(); // Create a new process

    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }
    else if (pid == 0)
    {
        // This block runs in the child process
        printf("Child Process: PID = %d, Parent PID = %d\n", getpid(), getppid());
    }
    else
    {
        // This block runs in the parent process
        printf("Parent Process: PID = %d, Child PID = %d\n", getpid(), pid);
    }

    printf("Process %d exiting...\n", getpid());
    return 0;
}