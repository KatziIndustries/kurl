#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "http.h"
#include "tcp.h"


size_t https_request(const char *host, const char *path, char *buffer, size_t buflen)
{
    SOCKET fd = tcp_connect(host, 443);

    if (fd == INVALID_SOCKET) {
        fprintf(stderr, "TCP connection failed\n");
        return 0;
    }

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());

    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        tcp_cleanup(fd);
        return 0;
    }

    SSL_CTX_set_verify(
        ctx,
        SSL_VERIFY_PEER,
        NULL
    );

    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        ERR_print_errors_fp(stderr);

        SSL_CTX_free(ctx);
        tcp_cleanup(fd);

        return 0;
    }

    SSL *ssl = SSL_new(ctx);

    if (ssl == NULL) {
        ERR_print_errors_fp(stderr);

        SSL_CTX_free(ctx);
        tcp_cleanup(fd);

        return 0;
    }

    if (SSL_set_tlsext_host_name(ssl, host) != 1) {
        ERR_print_errors_fp(stderr);

        SSL_free(ssl);
        SSL_CTX_free(ctx);
        tcp_cleanup(fd);

        return 0;
    }

    if (SSL_set1_host(ssl, host) != 1) {
        ERR_print_errors_fp(stderr);

        SSL_free(ssl);
        SSL_CTX_free(ctx);
        tcp_cleanup(fd);

        return 0;
    }

    if (SSL_set_fd(ssl, fd) != 1) {
        ERR_print_errors_fp(stderr);

        SSL_free(ssl);
        SSL_CTX_free(ctx);
        tcp_cleanup(fd);

        return 0;
    }

    if (SSL_connect(ssl) != 1) {
        ERR_print_errors_fp(stderr);

        SSL_free(ssl);
        SSL_CTX_free(ctx);
        tcp_cleanup(fd);

        return 0;
    }

    char request[4096];

    int request_len = snprintf(
        request,
        sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        path,
        host
    );

    if (request_len < 0 ||
        (size_t)request_len >= sizeof(request)) {

        fprintf(stderr, "request too large\n");

        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        tcp_cleanup(fd);

        return 0;
    }

    int written = 0;
    while (written < request_len) {
        int n = SSL_write(ssl, request + written, request_len - written);
        if (n <= 0) {
            ERR_print_errors_fp(stderr);

            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            tcp_cleanup(fd);

            return 0;
        }
        written += n;
    }

    size_t total_received = 0;

    while (1) {
        int n = SSL_read(
            ssl,
            buffer + total_received,
            (int)(buflen - 1 - total_received)
        );

        if (n > 0) {
            total_received += (size_t)n;
            continue;
        }

        int error = SSL_get_error(ssl, n);

        if (error == SSL_ERROR_ZERO_RETURN)
            break;

        if (error == SSL_ERROR_WANT_READ ||
            error == SSL_ERROR_WANT_WRITE)
            continue;

        ERR_print_errors_fp(stderr);
        break;
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    tcp_cleanup(fd);

    return total_received;
}
