#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>
#include <unistd.h>

static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
static int shared_counter = 0;
static atomic_ulong total_reader_operations = 0;
static atomic_ulong total_writer_operations = 0;
static volatile int stop_requested = 0;

typedef struct
{
    int thread_id;
} thread_arg_t;

static void do_busy_work(int iterations)
{
    volatile unsigned long temp = 0;
    for (int iterator = 0; iterator < iterations; ++iterator)
    {
        temp += iterator ^ (temp << 1);
    }
}

static void *reader_thread(void *arg)
{
    (void)arg;
    while (!stop_requested)
    {
        pthread_rwlock_rdlock(&rwlock);
        int local_value = shared_counter;
        (void)local_value;
        atomic_fetch_add(&total_reader_operations, 1);
        pthread_rwlock_unlock(&rwlock);
        do_busy_work(100);
    }
    return NULL;
}

static void *writer_thread(void *arg)
{
    (void)arg;
    while (!stop_requested)
    {
        pthread_rwlock_wrlock(&rwlock);
        shared_counter++;
        atomic_fetch_add(&total_writer_operations, 1);
        pthread_rwlock_unlock(&rwlock);
        do_busy_work(1000);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <num_readers> <num_writers> <runtime_seconds>\n", argv[0]);
        return 1;
    }

    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    int runtime_seconds = atoi(argv[3]);

    pthread_t reader_threads[num_readers];
    pthread_t writer_threads[num_writers];

    for (int iterator = 0; iterator < num_readers; ++iterator)
    {
        pthread_create(&reader_threads[iterator], NULL, reader_thread, NULL);
    }
    for (int iterator = 0; iterator < num_writers; ++iterator)
    {
        pthread_create(&writer_threads[iterator], NULL, writer_thread, NULL);
    }

    sleep(runtime_seconds);
    stop_requested = 1;

    for (int iterator = 0; iterator < num_readers; ++iterator)
    {
        pthread_join(reader_threads[iterator], NULL);
    }
    for (int iterator = 0; iterator < num_writers; ++iterator)
    {
        pthread_join(writer_threads[iterator], NULL);
    }

    printf("Total Reads:  %lu\n", (unsigned long)atomic_load(&total_reader_operations));
    printf("Total Writes: %lu\n", (unsigned long)atomic_load(&total_writer_operations));
    printf("Final shared_counter = %d\n", shared_counter);

    pthread_rwlock_destroy(&rwlock);
    return 0;
}