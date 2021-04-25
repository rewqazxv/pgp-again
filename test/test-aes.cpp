#include <gtest/gtest.h>

#include <iostream>
#include <random>
using namespace std;

extern "C" {
#include "../src/encrypt/aes.h"
}


Byte randbyte()
{
    static bool first = true;
    if (first) {
        srand(time(0));
        first = false;
    }
    return rand();
}

void printbuf(Byte *p, size_t len)
{
    for (int i = 0; i < len; i++)
        printf("%02X", (unsigned char)p[i]);
}

TEST(aes, 1)
{
    Byte data[512] = "Advanced Encryption Standard AES";
    size_t datalen = strlen((char *)data);
    Byte key[] = "Thats my Kung Fu";
    Byte chiper[512];
    printf("plain text: %s\n", data);
    printf("plain: "); printbuf(data, datalen); putchar('\n');
    printf("key: %s\n", key);

    aes128cbc_encrypt(chiper, data, datalen, key, randbyte);
    size_t chiperlen = aes128cbc_chiperlen(datalen);
    printf("chiper: "); printbuf(chiper, chiperlen); putchar('\n');

    memset(data, 0, sizeof(data));
    size_t decryptlen = aes128cbc_decrypt(data, chiper, chiperlen, key);
    printf("decrypted text: %s\n", data);
    printf("decrypted: "); printbuf(data, chiperlen - 16); putchar('\n');
}
