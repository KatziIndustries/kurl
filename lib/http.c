#include <stdio.h>
#include <string.h>

#include "tcp.h"

size_t http_request(const char* host, const char* path, char *buffer, size_t buflen)
{
    int port = 80;
    SOCKET sock = tcp_connect(host, port);

    if (sock == INVALID_SOCKET)
        return 0;

    char request[512];
    int request_len = snprintf(
        request,
        sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: Katzi/0.1\r\n"
        "Connection: close\r\n"
        "\r\n",
        path,
        host
    );

    if (request_len < 0 || (size_t)request_len >= sizeof(request)) {
        fprintf(stderr, "HTTP request too large\n");
        tcp_cleanup(sock);
        return 0;
    }

    if (tcp_send(sock, request, (size_t)request_len) < 0) {
        tcp_cleanup(sock);
        return 0;
    }

    size_t total_received = 0;

    while (total_received < buflen - 1) {
        int n = tcp_receive(
            sock,
            buffer + total_received,
            buflen - 1 - total_received
        );

        if (n <= 0)
            break;

        total_received += (size_t)n;
    }

    tcp_cleanup(sock);
    return total_received;
}
