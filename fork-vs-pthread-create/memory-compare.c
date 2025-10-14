#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/wait.h>

// Global shared variable
int shared_var = 100;

// Thread function: increments shared_var by 50
void *thread_func(void *arg)
{
    shared_var += 50; // Modify shared variable
    printf("Inside thread: shared_var = %d\n", shared_var);
    return NULL;
}

int main()
{
    // Print initial value of shared_var before any process/thread
    printf("Before fork: shared_var = %d\n", shared_var);

    // Create a new process using fork()
    pid_t pid = fork();
    if (pid < 0)
    {
        // fork() failed
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // Child process executes this block
        shared_var += 10; // Modify its own copy of shared_var
        printf("Child (fork): shared_var = %d\n", shared_var);
        exit(EXIT_SUCCESS); // Exit child process
    }
    else
    {
        // Parent process executes this block
        if (wait(NULL) < 0)
        { // Wait for child to finish
            perror("wait failed");
            exit(EXIT_FAILURE);
        }
        printf("Parent (after fork): shared_var = %d\n", shared_var);

        // Create a new thread in the parent process
        pthread_t t;
        int ret;

        // Create thread and check for errors
        ret = pthread_create(&t, NULL, thread_func, NULL);
        if (ret != 0)
        {
            fprintf(stderr, "Error creating thread: %d\n", ret);
            exit(EXIT_FAILURE);
        }

        // Wait for thread to finish and check for errors
        ret = pthread_join(t, NULL);
        if (ret != 0)
        {
            fprintf(stderr, "Error joining thread: %d\n", ret);
            exit(EXIT_FAILURE);
        }

        // Print shared_var after thread execution
        printf("After thread: shared_var = %d\n", shared_var);
    }

    return 0;
}