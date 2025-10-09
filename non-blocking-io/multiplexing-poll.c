#include <stdio.h>  // printf(), perror()
#include <stdlib.h> // exit(), EXIT_FAILURE
#include <unistd.h> // read(), close()
#include <fcntl.h>  // open(), O_RDONLY
#include <poll.h>   // poll(), struct pollfd, POLLIN

int main(void)
{
    int fd;
    char buffer[100]; // Buffer to store input from stdin or file

    // Open the file "test.txt" in read-only mode
    fd = open("test.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("open"); // Print error if file cannot be opened
        return 1;
    }

    // Create an array of pollfd structures to monitor multiple FDs
    struct pollfd fds[2];

    // Monitor keyboard input (stdin)
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN; // Wait for data to read

    // Monitor file descriptor (test.txt)
    fds[1].fd = fd;
    fds[1].events = POLLIN; // Wait for data to read

    printf("Monitoring stdin and file for input using poll()...\n");

    while (1)
    {
        // Poll indefinitely (-1 means no timeout) until at least one FD is ready
        int ret = poll(fds, 2, -1);

        if (ret < 0)
        {
            perror("poll"); // Print error if poll fails
            break;
        }

        // Check if stdin has data ready to read
        if (fds[0].revents & POLLIN)
        {
            int n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0'; // Null-terminate string
                printf("You typed: %s\n", buffer);
            }
        }

        // Check if the file has data ready to read
        if (fds[1].revents & POLLIN)
        {
            int n = read(fd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0'; // Null-terminate string
                printf("File content: %s\n", buffer);
            }
        }
    }

    // Close the file descriptor before exiting
    close(fd);

    return 0; // Successful exit
}