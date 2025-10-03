/*
Unnamed pipe = private, temporary communication between related processes.
Named pipe = public, persistent communication channel accessible by any process.
*/

// Creating a FIFO
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

int main()
{
    const char *fifoPath = "myfifo";

    // Create FIFO with read & write permissions
    if (mkfifo(fifoPath, 0666) == -1)
    {
        perror("mkfifo failed");
        return 1;
    }

    printf("FIFO created at %s\n", fifoPath);
    return 0;
}