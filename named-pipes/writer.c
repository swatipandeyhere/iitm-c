#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd = open("myfifo", O_WRONLY);
    if (fd == -1)
    {
        perror("open failed");
        return 1;
    }

    char message[] = "Hello from writer process!";
    write(fd, message, sizeof(message));
    printf("Writer sent: %s\n", message);

    close(fd);
    return 0;
}