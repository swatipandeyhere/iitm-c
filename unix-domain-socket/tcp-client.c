/*
 * TCP Client
 *
 * Connects to 127.0.0.1:9000,
 * receives greeting, sends a line,
 * prints reply.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUF_SIZE 1024

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
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
        die("socket");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        die("connect");

    char buf[BUF_SIZE + 1];
    ssize_t n = read_all_or_until_block(fd, buf, BUF_SIZE);
    if (n < 0)
        die("read(greeting)");
    buf[n] = '\0';
    fprintf(stderr, "[tcp-client] server says: %s", buf);

    const char *line = "We are learning TCP sockets!\n";
    if (write(fd, line, strlen(line)) < 0)
        die("write");

    n = read_all_or_until_block(fd, buf, BUF_SIZE);
    if (n < 0)
        die("read(reply)");
    buf[n] = '\0';
    fprintf(stderr, "[tcp-client] reply: %s", buf);

    close(fd);
    return 0;
}