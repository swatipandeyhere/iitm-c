/*
 * UDS Client Program
 * ------------------
 * This client connects to a Unix Domain Socket (UDS) server,
 * receives a greeting message, sends a line of text,
 * and prints the server's reply.
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/uds-demo.sock" // Path to the Unix domain socket
#define BUF_SIZE 1024                    // Buffer size for reading/writing data

/*
 * Utility function to print error message and exit the program.
 */
static void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

/*
 * Reads all available data from a file descriptor (fd) until either:
 *  - the buffer is full, or
 *  - a short read (<128 bytes) indicates the sender likely paused, or
 *  - EOF/error occurs.
 */
static ssize_t read_all_or_until_block(int fd, char *buf, size_t cap)
{
    ssize_t total = 0;

    for (;;)
    {
        ssize_t r = read(fd, buf + total, cap - (size_t)total);

        if (r > 0)
        {
            total += r;
            if ((size_t)total == cap)
                return total; // Buffer is full

            // Heuristic: stop reading if the last read was small (<128 bytes)
            if (r < 128)
                return total;
        }
        else if (r == 0)
        {
            // End of file — peer closed connection
            return total;
        }
        else
        {
            // Handle temporary read interruptions
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return total;

            // Any other error
            return -1;
        }
    }
}

int main(void)
{
    /* Step 1: Create a Unix domain socket */
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
        die("socket(AF_UNIX)");

    /* Step 2: Prepare the socket address structure */
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    /* Step 3: Connect to the server */
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        die("connect");

    /* Step 4: Read greeting message from server */
    char buf[BUF_SIZE + 1];
    ssize_t n = read_all_or_until_block(fd, buf, BUF_SIZE);
    if (n < 0)
        die("read(greeting)");
    buf[n] = '\0';
    fprintf(stderr, "[client] server says: %s", buf);

    /* Step 5: Send a message to the server */
    const char *line = "We are learning UDS!\n";
    if (write(fd, line, strlen(line)) < 0)
        die("write");

    /* Step 6: Read the server’s reply */
    n = read_all_or_until_block(fd, buf, BUF_SIZE);
    if (n < 0)
        die("read(reply)");
    buf[n] = '\0';
    fprintf(stderr, "[client] reply: %s", buf);

    /* Step 7: Close the connection */
    close(fd);

    return 0;
}