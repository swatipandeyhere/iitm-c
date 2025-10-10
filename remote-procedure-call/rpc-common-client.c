// rpc-common-client.c
// -----------------------------------------------------------------------------
// Example client demonstrating structured RPC communication using rpc-common.h
// Sends a RpcRequest struct and receives a RpcResponse struct.
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
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert address and connect
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Prepare request
    RpcRequest req;
    RpcResponse res;
    memset(&req, 0, sizeof(req));
    memset(&res, 0, sizeof(res));

    printf("Select operation (1=ADD, 2=SUB, 3=MUL, 4=DIV): ");
    scanf("%d", &req.op);
    printf("Enter first number: ");
    scanf("%lf", &req.a);
    printf("Enter second number: ");
    scanf("%lf", &req.b);

    // Send structured request
    if (write(sock, &req, sizeof(req)) != sizeof(req))
    {
        perror("write failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Receive structured response
    ssize_t bytes_read = read(sock, &res, sizeof(res));
    if (bytes_read != sizeof(res))
    {
        perror("read failed or incomplete");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Display result
    if (res.status == 0)
        printf("Server Result: %.2lf\n", res.result);
    else
        printf("Server Error: %s\n", res.message);

    close(sock);
    return 0;
}