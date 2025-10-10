// rpc-common.h
// -----------------------------------------------------------------------------
// Shared RPC definitions for request/response structures and operation codes.
// This header simulates what frameworks like gRPC generate automatically.
// -----------------------------------------------------------------------------

#ifndef RPC_COMMON_H
#define RPC_COMMON_H

// Operation codes for basic arithmetic RPC operations
#define OP_ADD 1
#define OP_SUB 2
#define OP_MUL 3
#define OP_DIV 4

// Structure representing a client request
typedef struct
{
    int op;   // Operation type: OP_ADD, OP_SUB, etc.
    double a; // Operand 1
    double b; // Operand 2
} RpcRequest;

// Structure representing a server response
typedef struct
{
    int status;       // 0 = OK, 1 = ERROR
    double result;    // Computed result if OK
    char message[64]; // Optional message or error description
} RpcResponse;

#endif // RPC_COMMON_H