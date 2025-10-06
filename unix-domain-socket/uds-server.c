/*
 * UDS (Unix Domain Socket) Server
 *
 * Steps:
 * 1. Create a socket endpoint - socket()
 * 2. Bind to file path - bind()
 * 3. Start listening - listen()
 * 4. Accept client - accept()
 * 5. Read client message - read() or read_line()
 * 6. Process message - convert to uppercase
 * 7. Write reply - write_all()
 * 8. Close client - close()
 * 9. Cleanup - unlink() or atexit()
 *
 * Linux only: prints client credentials using SO_PEERCRED
 */

#define _GNU_SOURCE // Needed for SO_PEERCRED on Linux

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/uds-demo.sock" // Socket file path
#define BACKLOG 10                       // Max pending connections
#define BUF_SIZE 1024                    // Max client message size

static int listen_fd = -1; // Global listening socket descriptor

/*------------------------------------------------
  Error handler: prints message and exits program
-------------------------------------------------*/
static void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

/*------------------------------------------------
  Cleanup function called at program exit
  - Closes listening socket
  - Removes socket file
-------------------------------------------------*/
static void cleanup(void)
{
    if (listen_fd != -1)
        close(listen_fd);
    unlink(SOCKET_PATH);
}

/*------------------------------------------------
  Signal handler for SIGINT/SIGTERM
  - Exits program, triggers atexit(cleanup)
-------------------------------------------------*/
static void on_signal(int sig)
{
    (void)sig; // unused
    exit(0);
}

/*------------------------------------------------
  Read a line from socket
  - Stops at newline or maxlen-1
  - Returns number of bytes read
  - Adds null terminator
-------------------------------------------------*/
static ssize_t read_line(int fd, char *buf, size_t maxlen)
{
    size_t n = 0;
    while (n + 1 < maxlen)
    {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r == 0)
            break; // EOF
        if (r < 0)
        {
            if (errno == EINTR)
                continue; // Interrupted -> retry
            return -1;    // Error
        }

        buf[n++] = c;
        if (c == '\n')
            break;
    }
    buf[n] = '\0'; // Null-terminate
    return (ssize_t)n;
}

/*------------------------------------------------
  Convert string to uppercase in place
-------------------------------------------------*/
static void to_upper(char *s)
{
    for (; *s; ++s)
    {
        if (*s >= 'a' && *s <= 'z')
            *s = (char)(*s - 'a' + 'A');
    }
}

/*------------------------------------------------
  Write all bytes to socket
  - Handles partial writes
  - Returns 0 on success, -1 on error
-------------------------------------------------*/
static int write_all(int fd, const void *buf, size_t len)
{
    const char *p = (const char *)buf;
    while (len > 0)
    {
        ssize_t w = write(fd, p, len);
        if (w < 0)
        {
            if (errno == EINTR)
                continue; // Interrupted -> retry
            return -1;    // Error
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
        fprintf(stderr, "[server] peer pid=%d uid=%d gid=%d\n", cred.pid, cred.uid, cred.gid);
    }
    else
    {
        perror("getsockopt(SO_PEERCRED)");
    }
#else
    (void)client_fd; // Prevent unused variable warning
    fprintf(stderr, "[server] peer credential fetch not implemented on this OS\n");
#endif
}

/*------------------------------------------------
  Main server function
-------------------------------------------------*/
int main(void)
{
    /* Register cleanup function for exit */
    atexit(cleanup);

    /* Setup signal handlers */
    struct sigaction sa = {0};
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* Restrict default permissions of socket file */
    umask(077);

    /* Create listening socket */
    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd == -1)
        die("socket(AF_UNIX)");

    /* Setup socket address */
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    /* Remove stale socket file */
    unlink(SOCKET_PATH);

    /* Bind socket to path */
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        die("bind");

    /* Restrict socket permissions (owner only) */
    if (chmod(SOCKET_PATH, 0600) == -1)
        die("chmod(socket)");

    /* Start listening */
    if (listen(listen_fd, BACKLOG) == -1)
        die("listen");

    fprintf(stderr, "[server] listening on %s\n", SOCKET_PATH);

    /*-----------------------------------------
      Main server loop: accept and handle clients
    ------------------------------------------*/
    for (;;)
    {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd == -1)
        {
            if (errno == EINTR)
                continue; // Interrupted -> retry
            die("accept");
        }

        /* Print client credentials */
        print_peer_credentials(client_fd);

        /* Send greeting to client */
        const char *greet = "Hello! You’re connected to the UDS Server. Send a line, and I’ll convert it to uppercase.\n";
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