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
    int server_fd, client_fd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    socklen_t addrlen = sizeof(address);

    // Step 1: Create TCP socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Step 2: Bind socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Step 3: Listen
    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("RPC Server running on port %d...\n", PORT);

    // Step 4: Accept one client
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Client connected!\n");

    // Step 5: Read client message
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read < 0)
    {
        perror("read failed");
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    buffer[bytes_read] = '\0';
    printf("Received: %s\n", buffer);

    // Step 6: Parse command
    char command[10];
    double a, b, result = 0.0;
    int matched = sscanf(buffer, "%s %lf %lf", command, &a, &b);

    char response[100];
    if (matched == 3)
    {
        if (strcmp(command, "ADD") == 0)
            result = a + b;
        else if (strcmp(command, "SUB") == 0)
            result = a - b;
        else if (strcmp(command, "MUL") == 0)
            result = a * b;
        else if (strcmp(command, "DIV") == 0)
        {
            if (b == 0)
                sprintf(response, "ERROR Division by zero");
            else
                result = a / b;
        }
        else
        {
            sprintf(response, "ERROR Unknown command");
        }

        if (strncmp(response, "ERROR", 5) != 0)
            sprintf(response, "RESULT %.2lf", result);
    }
    else
    {
        sprintf(response, "ERROR Invalid format");
    }

    // Step 7: Send response to client
    send(client_fd, response, strlen(response), 0);
    printf("Sent: %s\n", response);

    // Step 8: Close sockets
    close(client_fd);
    close(server_fd);
    return 0;
}