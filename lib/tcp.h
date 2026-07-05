#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <winsock2.h>
#elif __unix__
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
    #define SOCKET_ERROR -1
#else
    #error "Unknown platform"
#endif

SOCKET tcp_connect(const char *host, int port);

void tcp_cleanup(SOCKET sock);

int tcp_send(SOCKET sock, const char *data, size_t len);

int tcp_receive(SOCKET sock, char *buffer, size_t max_len);