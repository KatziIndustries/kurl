#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <netdb.h>
    #include <unistd.h>
    #include <sys/socket.h>
#endif

#ifndef _WIN32
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
#endif

void ErrExit(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

static int socket_startup(void)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return -1;
#endif
    return 0;
}

SOCKET tcp_connect(const char *host, int port)
{
    char service[16];
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    struct addrinfo *rp;
    SOCKET sock = INVALID_SOCKET;

    if (socket_startup() != 0) {
        fprintf(stderr, "WSAStartup() error!\n");
        return INVALID_SOCKET;
    }

    int service_len = snprintf(service, sizeof(service), "%d", port);
    if (service_len < 0 || (size_t)service_len >= sizeof(service))
        goto fail;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(host, service, &hints, &result);
    if (ret != 0) {
#ifdef _WIN32
        fprintf(stderr, "getaddrinfo() failed: %d\n", ret);
#else
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(ret));
#endif
        goto fail;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == INVALID_SOCKET)
            continue;

        if (connect(sock, rp->ai_addr, (int)rp->ai_addrlen) == 0)
            break;

        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "connect() failed for %s:%d\n", host, port);
#ifdef _WIN32
        WSACleanup();
#endif
    }

    return sock;

fail:
    if (result != NULL)
        freeaddrinfo(result);
#ifdef _WIN32
    WSACleanup();
#endif
    return INVALID_SOCKET;
}

int tcp_send(SOCKET sock, const char *data, size_t len)
{
    size_t total_sent = 0;

    while (total_sent < len) {
        size_t remaining = len - total_sent;
        int chunk = remaining > (size_t)INT_MAX ? INT_MAX : (int)remaining;
        int sent = send(sock, data + total_sent, chunk, 0);

        if (sent == SOCKET_ERROR) {
            perror("send");
            return -1;
        }
        if (sent == 0) {
            fprintf(stderr, "send() returned 0\n");
            return -1;
        }

        total_sent += (size_t)sent;
    }

    return total_sent > (size_t)INT_MAX ? -1 : (int)total_sent;
}

int tcp_receive(SOCKET sock, char *buffer, size_t max_len)
{
    if (max_len < 2)
        return -1;

    size_t usable = max_len - 1;
    int chunk = usable > (size_t)INT_MAX ? INT_MAX : (int)usable;
    int received = recv(sock, buffer, chunk, 0);

    if (received == SOCKET_ERROR) {
        perror("recv");
        return -1;
    }

    if (received == 0)
        return 0;

    buffer[received] = '\0';
    return received;
}

void tcp_cleanup(SOCKET sock)
{
    if (sock != INVALID_SOCKET) {
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
    }
}
