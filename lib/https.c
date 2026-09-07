#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "http.h"

static int tcp_connect(const char *host, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result;
    struct addrinfo *rp;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int ret = getaddrinfo(host, port, &hints, &result);

    if (ret != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
    }

    int fd = -1;

    for (rp = result; rp != NULL; rp = rp->ai_next) {

        fd = socket(
            rp->ai_family,
            rp->ai_socktype,
            rp->ai_protocol
        );

        if (fd == -1)
            continue;

        if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        close(fd);
        fd = -1;
    }

    freeaddrinfo(result);

    return fd;
}

void https_request(const char *host, const char *path)
{
    int fd = tcp_connect(host, "443");

    if (fd == -1) {
        fprintf(stderr, "TCP connection failed\n");
        return;
    }

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());

    if (ctx == NULL) {
        ERR_print_errors_fp(stderr);
        close(fd);
        return;
    }

    /*
     * Zertifikate überprüfen
     */
    SSL_CTX_set_verify(
        ctx,
        SSL_VERIFY_PEER,
        NULL
    );

    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        ERR_print_errors_fp(stderr);

        SSL_CTX_free(ctx);
        close(fd);

        return;
    }

    SSL *ssl = SSL_new(ctx);

    if (ssl == NULL) {
        ERR_print_errors_fp(stderr);

        SSL_CTX_free(ctx);
        close(fd);

        return;
    }

    /*
     * SNI
     *
     * Wichtig bei HTTPS-Servern mit mehreren Domains
     * auf derselben IP.
     */
    if (SSL_set_tlsext_host_name(ssl, host) != 1) {
        ERR_print_errors_fp(stderr);

        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(fd);

        return;
    }

    /*
     * Hostname für Zertifikatsprüfung setzen
     */
    if (SSL_set1_host(ssl, host) != 1) {
        ERR_print_errors_fp(stderr);

        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(fd);

        return;
    }

    /*
     * TCP Socket an OpenSSL hängen
     */
    if (SSL_set_fd(ssl, fd) != 1) {
        ERR_print_errors_fp(stderr);

        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(fd);

        return;
    }

    /*
     * TLS Handshake
     */
    if (SSL_connect(ssl) != 1) {
        ERR_print_errors_fp(stderr);

        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(fd);

        return;
    }

    printf(
        "TLS connected: %s\n",
        SSL_get_version(ssl)
    );

    /*
     * HTTP Request
     */
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

    if (request_len < 0 || (size_t)request_len >= sizeof(request)) {
        fprintf(stderr, "request too large\n");

        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(fd);

        return;
    }

    /*
     * HTTP → TLS → TCP
     */
    if (SSL_write(ssl, request, request_len) <= 0) {
        ERR_print_errors_fp(stderr);

        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(fd);

        return;
    }

    /*
     * Response
     */
    char buffer[8192];

    while (1) {
        int n = SSL_read(
            ssl,
            buffer,
            sizeof(buffer)
        );

        if (n > 0) {
            /*
             * Hier deinen vorhandenen HTTP Parser verwenden.
             */
            fwrite(buffer, 1, n, stdout);
            continue;
        }

        int error = SSL_get_error(ssl, n);

        if (error == SSL_ERROR_ZERO_RETURN)
            break;

        ERR_print_errors_fp(stderr);
        break;
    }

    SSL_shutdown(ssl);

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(fd);
}
