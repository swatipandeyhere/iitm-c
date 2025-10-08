#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

int main(void)
{
    int fd;
    char buffer[100];

    // Open the file in read-only mode
    fd = open("test.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    // Make the file descriptor non-blocking
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    // Make stdin non-blocking as well
    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    fd_set readfds;
    int maxfd = (fd > STDIN_FILENO ? fd : STDIN_FILENO) + 1;

    printf("Monitoring stdin and file for input in non-blocking mode...\n");

    while (1)
    {
        // Clear the fd set and add the descriptors
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(fd, &readfds);

        // Set timeout to 0 for non-blocking select
        struct timeval tv = {0, 0};

        int activity = select(maxfd, &readfds, NULL, NULL, &tv);
        if (activity < 0)
        {
            perror("select");
            break;
        }

        // Check if stdin is ready
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            int n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0';
                printf("You typed: %s\n", buffer);
            }
            else if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("read stdin");
            }
        }

        // Check if file descriptor is ready
        if (FD_ISSET(fd, &readfds))
        {
            int n = read(fd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0';
                printf("File content: %s\n", buffer);
            }
            else if (n == 0)
            {
                printf("Reached end of file.\n");
                break; // Exit loop after reading the file
            }
            else if (n == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("read file");
                break;
            }
        }

        // Optional: small sleep to avoid busy-waiting
        usleep(10000); // 10 ms
    }

    close(fd);
    return 0;
}