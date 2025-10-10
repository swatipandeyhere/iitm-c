#include <stdio.h>      // For printf, perror
#include <stdlib.h>     // For exit()
#include <string.h>     // For memset(), strlen()
#include <unistd.h>     // For close()
#include <arpa/inet.h>  // For htons(), inet_ntoa(), etc.
#include <sys/socket.h> // For socket(), bind(), listen(), accept()

#define PORT 8080 // TCP port to listen on
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Step 1: Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Step 2: Set socket options (macOS safe)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

#ifdef SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(SO_REUSEPORT) failed");
        // not fatal — continue
    }
#endif

    // Step 3: Bind socket to all interfaces and PORT
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    address.sin_port = htons(PORT);       // Convert port to network byte order

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Step 4: Listen for incoming connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    // Step 5: Accept one client
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Client connected!\n");

    // Step 6: Read message from client
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0)
    {
        perror("read failed");
    }
    else
    {
        buffer[bytes_read] = '\0'; // Null-terminate
        printf("Received from client: %s\n", buffer);
    }

    // Step 7: Send reply to client
    const char *reply = "Hello from server!";
    send(client_fd, reply, strlen(reply), 0);
    printf("Reply sent to client.\n");

    // Step 8: Close sockets
    close(client_fd);
    close(server_fd);
    return 0;
}