#include <stdio.h>
#include <string.h>

#include "tcp.h"

int http_request(const char* host, const char* path)
{
    int port = 80;
    SOCKET sock = tcp_connect(host, port);

    if (sock == INVALID_SOCKET)
        return 1;

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
        return 1;
    }

    if (tcp_send(sock, request, (size_t)request_len) < 0) {
        tcp_cleanup(sock);
        return 1;
    }

    char response[1024];
    size_t total_received = 0;
    while (1) {
        int n = tcp_receive(sock, response, sizeof(response));
        if (n <= 0)
            break;

        total_received += (size_t)n;
        fwrite(response, 1, (size_t)n, stdout);
    }

    printf("\n\nTotal received: %zu bytes\n", total_received);
    tcp_cleanup(sock);
    return 0;
}
