#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"
#include "https.h"

typedef struct {
    char protocol[8];
    char host[256];
    char path[2048];
    char query[2048];
} katzi_url_t;

static int katzi_parse_url(const char *url, katzi_url_t *out)
{
    const char *p;
    const char *path;
    const char *query;
    size_t len;

    if (url == NULL || out == NULL)
        return -1;

    memset(out, 0, sizeof(*out));

    p = strstr(url, "://");
    if (p != NULL) {
        len = (size_t)(p - url);
        if (len == 0 || len >= sizeof(out->protocol))
            return -1;

        memcpy(out->protocol, url, len);
        out->protocol[len] = '\0';
        url = p + 3;
    } else {
        strcpy(out->protocol, "http");
    }

    path = strpbrk(url, "/?#");
    if (path == NULL) {
        if (strlen(url) >= sizeof(out->host))
            return -1;

        strcpy(out->host, url);
        strcpy(out->path, "/");
        return out->host[0] != '\0' ? 0 : -1;
    }

    len = (size_t)(path - url);
    if (len == 0 || len >= sizeof(out->host))
        return -1;

    memcpy(out->host, url, len);
    out->host[len] = '\0';

    if (*path == '/') {
        query = strpbrk(path, "?#");
        if (query == NULL) {
            if (strlen(path) >= sizeof(out->path))
                return -1;
            strcpy(out->path, path);
        } else {
            len = (size_t)(query - path);
            if (len >= sizeof(out->path))
                return -1;
            memcpy(out->path, path, len);
            out->path[len] = '\0';
        }
    } else {
        strcpy(out->path, "/");
    }

    query = strchr(path, '?');
    if (query != NULL) {
        const char *fragment;
        query++;
        fragment = strchr(query, '#');
        len = fragment != NULL ? (size_t)(fragment - query) : strlen(query);

        if (len >= sizeof(out->query))
            return -1;

        memcpy(out->query, query, len);
        out->query[len] = '\0';
    }

    return 0;
}

void katzi_url(const char *url)
{
    katzi_url_t parsed;
    char target[4096];
    int target_len;

    if (katzi_parse_url(url, &parsed) != 0) {
        fprintf(stderr, "invalid URL: %s\n", url != NULL ? url : "(null)");
        return;
    }

    if (parsed.query[0] != '\0') {
        target_len = snprintf(
            target,
            sizeof(target),
            "%s?%s",
            parsed.path,
            parsed.query
        );
    } else {
        target_len = snprintf(
            target,
            sizeof(target),
            "%s",
            parsed.path
        );
    }

    if (target_len < 0 || (size_t)target_len >= sizeof(target)) {
        fprintf(stderr, "URL target too large\n");
        return;
    }

    if (strcmp(parsed.protocol, "http") == 0) {
        http_request(parsed.host, target);
        return;
    }

    if (strcmp(parsed.protocol, "https") == 0) {
        https_request(parsed.host, target);
        return;
    }

    fprintf(stderr, "unsupported protocol: %s\n", parsed.protocol);
}
