#include <stdio.h>      // printf(), perror()
#include <stdlib.h>     // exit(), EXIT_FAILURE
#include <unistd.h>     // read(), close()
#include <sys/select.h> // select(), fd_set, FD_ZERO, FD_SET, FD_ISSET
#include <fcntl.h>      // open(), O_RDONLY

int main(void)
{
    int fd;
    char buffer[100]; // Buffer to store input from stdin or file

    // Open the file "test.txt" in read-only mode
    fd = open("test.txt", O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1; // Exit if file cannot be opened
    }

    // fd_set: used to store file descriptors to monitor for readability
    fd_set readfds;

    // Determine the maximum file descriptor value + 1 for select()
    int maxfd = (fd > STDIN_FILENO ? fd : STDIN_FILENO) + 1;

    printf("Monitoring stdin and file for input...\n");

    while (1)
    {
        // Clear the fd_set before each select() call
        FD_ZERO(&readfds);

        // Add stdin (keyboard) to the set
        FD_SET(STDIN_FILENO, &readfds);

        // Add file descriptor to the set
        FD_SET(fd, &readfds);

        // Block until at least one fd is ready for reading
        int activity = select(maxfd, &readfds, NULL, NULL, NULL);

        if (activity < 0)
        {
            perror("select"); // Print error if select fails
            break;
        }

        // Check if stdin has data to read
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            int n = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0'; // Null-terminate input
                printf("You typed: %s\n", buffer);
            }
        }

        // Check if the file has data to read
        if (FD_ISSET(fd, &readfds))
        {
            int n = read(fd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = '\0'; // Null-terminate file content
                printf("File content: %s\n", buffer);
            }
        }
    }

    // Close the file descriptor before exiting
    close(fd);

    return 0; // Successful exit
}