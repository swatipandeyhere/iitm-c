#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    // Step 1: Create TCP socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Step 2: Configure server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Step 3: Convert IP to numeric form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    // Step 4: Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Step 5: Send message to server
    const char *message = "Hello from client!";
    send(sock, message, strlen(message), 0);
    printf("Message sent to server.\n");

    // Step 6: Receive reply from server
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(sock, buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0)
    {
        perror("read failed");
    }
    else
    {
        buffer[bytes_read] = '\0'; // Null-terminate
        printf("Received from server: %s\n", buffer);
    }

    // Step 7: Close socket
    close(sock);
    return 0;
}