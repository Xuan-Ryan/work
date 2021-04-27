#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>

#define KGRN "\033[0;32;32m"
#define RESET "\033[0m"

char *printf_md5(unsigned char *in)
{

    int i;
    for (i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        if (in[i] < 0x10)
            printf("0%1x", in[i]);
        else
            printf("%2x", in[i]);
    }

    printf("\n" RESET);
    return in;
}

int main(void)
{
    char *message;
    const char *path = "Hello Ni hao ma?";

    unsigned char digest[MD5_DIGEST_LENGTH];

    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, path, strlen(path));
    MD5_Final(digest, &context);

    printf(KGRN "path '%s' md5:", path);

    printf_md5(digest);
    message = printf_md5(digest);
    //printf("%s\n", message);
    //printf_md5(message);

    return 0;
}
