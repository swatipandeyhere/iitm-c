// rpc-common-server.c
// -----------------------------------------------------------------------------
// Example server demonstrating structured RPC communication using rpc-common.h
// Client sends RpcRequest struct; server computes result and returns RpcResponse.
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "rpc-common.h"

#define PORT 8080

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Allow reuse of address/port (helps on macOS)
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configure address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("RPC Struct Server running on port %d...\n", PORT);

    // Accept one client
    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_fd < 0)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Client connected!\n");

    RpcRequest req;
    RpcResponse res = {0};

    // Read structured request from client
    ssize_t bytes_read = read(client_fd, &req, sizeof(req));
    if (bytes_read != sizeof(req))
    {
        perror("read failed or incomplete");
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Received request: op=%d, a=%.2lf, b=%.2lf\n", req.op, req.a, req.b);

    // Process request
    switch (req.op)
    {
    case OP_ADD:
        res.result = req.a + req.b;
        break;
    case OP_SUB:
        res.result = req.a - req.b;
        break;
    case OP_MUL:
        res.result = req.a * req.b;
        break;
    case OP_DIV:
        if (req.b == 0)
        {
            res.status = 1;
            snprintf(res.message, sizeof(res.message), "ERROR Division by zero");
        }
        else
        {
            res.result = req.a / req.b;
        }
        break;
    default:
        res.status = 1;
        snprintf(res.message, sizeof(res.message), "ERROR Unknown operation");
    }

    // Send structured response
    if (write(client_fd, &res, sizeof(res)) != sizeof(res))
    {
        perror("write failed");
    }
    else
    {
        if (res.status == 0)
            printf("Sent result: %.2lf\n", res.result);
        else
            printf("Sent error: %s\n", res.message);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}