#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

#define BUF_SIZE 100

char *read_fifo = "/tmp/fifo2";
char *write_fifo = "/tmp/fifo1";

void *read_messages(void *arg)
{
    char buf[BUF_SIZE];
    int fd;

    while (1)
    {
        fd = open(read_fifo, O_RDONLY);
        if (fd < 0)
            perror("open read_fifo");

        read(fd, buf, BUF_SIZE);
        printf("\nUser2: %s\nYou: ", buf);
        fflush(stdout);
        close(fd);
    }
}

int main()
{
    pthread_t tid;

    mkfifo(write_fifo, 0666);
    mkfifo(read_fifo, 0666);

    pthread_create(&tid, NULL, read_messages, NULL);

    char buf[BUF_SIZE];
    int fd;

    while (1)
    {
        printf("You: ");
        fgets(buf, BUF_SIZE, stdin);

        fd = open(write_fifo, O_WRONLY);
        if (fd < 0)
            perror("open write_fifo");

        write(fd, buf, strlen(buf) + 1);
        close(fd);
    }

    pthread_join(tid, NULL);
    return 0;
}