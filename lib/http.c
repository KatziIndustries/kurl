#include "tcp.h"

int http_request(const char* host,const char* path)
{
    int port = 80;

    SOCKET sock = tcp_connect(host,port);

    if (sock == INVALID_SOCKET)
    {
        tcp_cleanup(sock);
        return 1;
    }

    char request[512];
    snprintf(
        request,
        sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: Mozilla/5.0 (macOS; AArch64) Katzi/0.1 Chrome/146.0.0.0 AppleWebKit/537.36 Safari/537.36"
        "Connection: close\r\n"
        "\r\n",
        path,
        host
    );
    
    if (tcp_send(sock, request, strlen(request)) < 0)
    {
        tcp_cleanup(sock);
        return 1;
    }

    char respone[1024];
    int total_received = 0;
    while (1)
    {
        int n = tcp_receive(sock, respone, 1024);
        if (n <= 0) break;
        
        total_received += n;
        printf("%s", respone);
    }

    printf("\n\nTotal received: %d bytes\n", total_received);
    tcp_cleanup(sock);
    return 0;
}
