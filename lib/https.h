#ifndef HTTPS_H
#define HTTPS_H

size_t https_request(const char *host, const char *path, char *buffer, size_t buflen);

#endif
