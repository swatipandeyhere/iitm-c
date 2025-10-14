#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// Worker thread function
void *worker(void *arg)
{
    int *num = (int *)arg; // Cast argument to int pointer
    printf("Worker thread started. Thread ID: %p, Argument: %d\n", (void *)pthread_self(), *num);

    sleep(1); // Simulate some work

    printf("Worker thread finishing.\n");
    return NULL;
}

int main()
{
    pthread_t thread; // Thread identifier
    int value = 42;   // Argument to pass to the worker thread

    printf("Main thread ID: %p\n", (void *)pthread_self());

    // Create a new thread running the worker function
    pthread_create(&thread, NULL, worker, &value);

    // Main thread continues execution while worker runs
    printf("Main thread continues while worker runs...\n");

    // Wait for the worker thread to finish
    pthread_join(thread, NULL);

    printf("Main thread done.\n");
    return 0;
}