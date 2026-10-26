#ifndef HTTP_H
#define HTTP_H

size_t http_request(const char* host,const char* path, char *buffer, size_t buflen);

#endif
