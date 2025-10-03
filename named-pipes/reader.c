#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    int fd = open("myfifo", O_RDONLY);
    if (fd == -1)
    {
        perror("open failed");
        return 1;
    }

    char buffer[100];
    read(fd, buffer, sizeof(buffer));
    printf("Reader received: %s\n", buffer);

    close(fd);
    return 0;
}