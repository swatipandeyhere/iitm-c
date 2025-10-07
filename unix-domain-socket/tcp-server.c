/*
 * TCP Server (localhost)
 *
 * Steps:
 * 1. Create a socket endpoint - socket()
 * 2. Bind to 127.0.0.1:9000 - bind()
 * 3. Start listening - listen()
 * 4. Accept client - accept()
 * 5. Read client message - read_line()
 * 6. Process message - convert to uppercase
 * 7. Write reply - write_all()
 * 8. Close client - close()
 *
 * Linux only: prints client credentials using SO_PEERCRED
 */

#define _GNU_SOURCE // Needed for SO_PEERCRED on Linux

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 9000
#define BACKLOG 10
#define BUF_SIZE 1024

static int listen_fd = -1;

/* Error handler */
static void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

/* Cleanup */
static void cleanup(void)
{
    if (listen_fd != -1)
        close(listen_fd);
}

/* Signal handler */
static void on_signal(int sig)
{
    (void)sig;
    exit(0);
}

/* Read a line from socket */
static ssize_t read_line(int fd, char *buf, size_t maxlen)
{
    size_t n = 0;
    while (n + 1 < maxlen)
    {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r == 0)
            break;
        if (r < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        buf[n++] = c;
        if (c == '\n')
            break;
    }
    buf[n] = '\0';
    return (ssize_t)n;
}

/* Convert string to uppercase */
static void to_upper(char *s)
{
    for (; *s; ++s)
    {
        if (*s >= 'a' && *s <= 'z')
            *s = (char)(*s - 'a' + 'A');
    }
}

/* Write all bytes */
static int write_all(int fd, const void *buf, size_t len)
{
    const char *p = (const char *)buf;
    while (len > 0)
    {
        ssize_t w = write(fd, p, len);
        if (w < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        p += w;
        len -= (size_t)w;
    }
    return 0;
}

/*------------------------------------------------
  Print peer credentials (Linux only)
  - pid, uid, gid of connected client
-------------------------------------------------*/
static void print_peer_credentials(int client_fd)
{
#ifdef __linux__
    struct ucred cred;
    socklen_t len = sizeof(cred);

    if (getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &cred, &len) == 0)
    {
        fprintf(stderr, "[tcp-server] peer pid=%d uid=%d gid=%d\n", cred.pid, cred.uid, cred.gid);
    }
    else
    {
        perror("getsockopt(SO_PEERCRED)");
    }
#else
    (void)client_fd; // Prevent unused variable warning
    fprintf(stderr, "[tcp-server] peer credential fetch not implemented on this OS\n");
#endif
}

/* Main server */
int main(void)
{
    atexit(cleanup);

    struct sigaction sa = {0};
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Create listening socket */
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
        die("socket");

    /* Setup socket address */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    /* Bind socket to local address (127.0.0.1:PORT) */
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        die("bind");

    /* Start listening */
    if (listen(listen_fd, BACKLOG) == -1)
        die("listen");

    fprintf(stderr, "[tcp-server] listening on 127.0.0.1:%d\n", PORT);

    /*-----------------------------------------
      Main server loop: accept and handle clients
    ------------------------------------------*/
    for (;;)
    {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd == -1)
        {
            if (errno == EINTR)
                continue;
            die("accept");
        }

        /* Print client credentials */
        print_peer_credentials(client_fd);

        /* Send greeting to client */
        const char *greet = "Hello! You’re connected to the TCP Server. Send a line, and I’ll convert it to uppercase.\n";
        if (write_all(client_fd, greet, strlen(greet)) < 0)
        {
            close(client_fd);
            continue;
        }

        /* Read client message */
        char buf[BUF_SIZE];
        ssize_t n = read_line(client_fd, buf, sizeof(buf));
        if (n <= 0)
        {
            close(client_fd);
            continue;
        }

        /* Convert message to uppercase */
        to_upper(buf);

        /* Prepare reply */
        char out[BUF_SIZE + 16]; // Extra space for "OK: " prefix
        int m = snprintf(out, sizeof(out), "OK: %s", buf);

        if (m < 0 || (size_t)m >= sizeof(out))
        {
            close(client_fd);
            continue;
        }

        /* Send reply to client */
        write_all(client_fd, out, (size_t)m);

        /* Close client socket */
        close(client_fd);
    }
}