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

void ErrExit(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

int create_server_socket(void)
{
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            ErrExit("WSAStartup() error!");
    #endif

    SOCKET serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == INVALID_SOCKET)
    {
        ErrExit("socket() error!");
    }

    return serv_sock;
}

SOCKET tcp_connect(const char *host, int port)
{
    SOCKET sock = create_server_socket();

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    struct hostent *server = gethostbyname(host);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR: No such host: %s\n", host);
        closesocket(sock);
        return INVALID_SOCKET;
    }
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) 
            == SOCKET_ERROR)
    {
        ErrExit("connect");
    }

    return sock;
}

int tcp_send(SOCKET sock, const char *data, size_t len)
{
    size_t total_sent = 0;
    while (total_sent < len)
    {
        int sent = send(sock, data + total_sent, (int)(len - total_sent), 0);
        if (sent == SOCKET_ERROR)
        {
            perror("send");
            return -1;
        }
        total_sent += sent;
    }
    return (int)total_sent;
}

int tcp_receive(SOCKET sock, char *buffer, size_t max_len)
{
    int received = recv(sock, buffer, (int)max_len - 1, 0);
    if (received == SOCKET_ERROR)
    {
        perror("recv");
        return -1;
    }
    if (received == 0)
    {
        return 0;
    }
    buffer[received] = '\0';
    return received;
}

void tcp_cleanup(SOCKET sock)
{
    #ifdef _WIN32
        closesocket(sock);
        WSACleanup();
    #else
        close(sock);
    #endif
}
