#include <stdio.h>  // For printf(), perror()
#include <unistd.h> // For read(), close(), etc.
#include <string.h> // For string operations (optional but useful)

int main()
{
    // Declare a buffer to hold user input
    // We reserve one extra byte for the null terminator '\0'
    char buffer[100];

    // Print an interactive prompt
    printf("Enter something: ");

    // Forcefully flush stdout to ensure the prompt appears immediately
    // Without this, stdout (which is line-buffered) may not display the message
    fflush(stdout);

    // Perform a blocking read operation
    // File descriptor 0 = standard input (stdin)
    // It will block until the user types something and presses Enter
    int n = read(0, buffer, sizeof(buffer) - 1);

    // Error handling: if read() returns -1, an error occurred
    if (n < 0)
    {
        perror("read"); // Prints a system-generated error message
        return 1;       // Non-zero return indicates failure
    }
    // If read() returns 0, it means end-of-file (EOF) — user pressed Ctrl+D
    else if (n == 0)
    {
        printf("\nNo input detected (EOF)\n");
        return 0; // Graceful exit
    }

    // Null-terminate the string so it can be safely used with printf("%s")
    buffer[n] = '\0';

    // Display what the user typed
    printf("You typed: %s\n", buffer);

    // Successful program termination
    return 0;
}