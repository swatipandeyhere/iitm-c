#include <stdio.h>  // printf(), fflush(), perror()
#include <unistd.h> // read(), usleep()
#include <fcntl.h>  // fcntl(), O_NONBLOCK, F_GETFL, F_SETFL
#include <string.h> // Optional: string manipulation
#include <errno.h>  // errno values like EAGAIN / EWOULDBLOCK

int main()
{
    char buffer[100]; // Buffer to store user input

    // Step 1: Get current flags of stdin (fd = 0)
    int flags = fcntl(0, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl get");
        return 1;
    }

    // Step 2: Set stdin to non-blocking mode
    if (fcntl(0, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl set");
        return 1;
    }

    // Print prompt to the user
    printf("Type something (non-blocking): ");
    fflush(stdout); // Ensure prompt appears immediately

    // Step 3: Non-blocking read loop
    while (1)
    {
        int n = read(0, buffer, sizeof(buffer) - 1);

        if (n > 0)
        {
            // Data available: null-terminate and print
            buffer[n] = '\0';
            printf("\nYou typed: %s\n", buffer);
            break;
        }
        else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            // No data available yet: program continues
            printf(".");    // Show responsive indicator
            fflush(stdout); // Print dot immediately
            usleep(200000); // Sleep 0.2 sec to reduce CPU usage
        }
        else
        {
            // Other errors
            perror("read");
            break;
        }
    }

    return 0; // Successful exit
}