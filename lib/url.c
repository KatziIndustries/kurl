#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"

void katzi_url(const char* url)
{
    char *request;

    if (strstr(url, "://") == NULL)
    {
        request = malloc(strlen("http://") + strlen(url) + 1);
        if (request == NULL)
        {
        }
    
        strcpy(request, "http://");
        strcat(request, url);
    }
    else
    {
        request = strdup(url);
        if (request == NULL)
        {
        }
    }

    char protocol[16] = "http";
    char host[128] = "";
    char pathname[128] = "/";
    char search[128] = "";
    char hash[128] = "";

    sscanf(
        request,
        "%15[^:]://%127[^/]%127[^?#]?%127[^#]#%127s",
        protocol,
        host,
        pathname,
        search,
        hash
    );

    if(strcmp(protocol,"http") == 0)
    {
        http_request(host,strcat(pathname,search));
    }
    if(strcmp(protocol,"https") == 0)
    {
        printf("no https support!\n");
    }
}
