#include <stdio.h>
#include <stdlib.h>

unsigned char TX_checksum(unsigned char *buf, unsigned char len) //buf為陣列，len為陣列長度
{
    unsigned char i, ret = 0;

    for (i = 0; i < len; i++)
    {
        ret += *(buf++);
    }
    ret = ~ret;
    return ret;
}
unsigned char RX_checksum(unsigned char *buf, unsigned char len)
{
    unsigned char i, ret = 0;

    for (i = 0; i < len; i++)
    {
        ret += *(buf++);
    }
    ret = ret;
    return ret + 1;
}

int main()
{
    unsigned char message[] = "hi this is chair";
    TX_checksum(message, sizeof(message));
    printf("%s\n", message);
    RX_checksum(message, sizeof(message));
}