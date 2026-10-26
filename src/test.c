#include <stdio.h>

#include "../lib/kurl.h"

int main(int argc, char *argv[])
{
    if(argc == 2)
    {
        char content[4096];
        size_t len = katzi_url(argv[1], content, 4096);
        fwrite(content, 1, len, stdout);
    }
}
