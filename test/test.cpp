#include <stdio.h>
#include <iostream>
#include <random>
#include <stdlib.h>
#include <string.h>
using namespace std;

#include "../encrypt/rsa.hpp"
extern "C" {
#include "../encrypt/aes.h"
}

Byte randbyte() {
    static bool first = true;
    if (first) {
        srand(time(0));
        first = false;
    }
    return rand();
}

void printbuf(Byte *p, size_t len) {
    for (int i=0;i<len;i++)
        printf("%02X", (unsigned char)p[i]);
}

void testaes() {
    puts("--------------------- AES ---------------------");
    Byte data[512] = "Advanced Encryption Standard AES";
    size_t datalen = strlen((char*)data);
    Byte key[] = "Thats my Kung Fu";
    Byte chiper[512];
    printf("plain text: %s\n", data);
    printf("plain: ");printbuf(data,datalen);putchar('\n');
    printf("key: %s\n", key);
    
    aes128cbc_encrypt(chiper, data, datalen, key, randbyte);
    size_t chiperlen = aes128cbc_chiperlen(datalen);
    printf("chiper: ");printbuf(chiper,chiperlen);putchar('\n');
    
    memset(data,0,sizeof(data));
    size_t decryptlen = aes128cbc_decrypt(data,chiper,chiperlen,key);
    printf("decrypted text: %s\n", data);
    printf("decrypted: ");printbuf(data,chiperlen-16);putchar('\n');
}

void testrsa() {
    puts("--------------------- RSA ---------------------");
    random_device rd;
    gmp_randclass mprand{gmp_randinit_mt};
    mprand.seed(rd());

    auto keys = rsa_keygen(mprand);
    cout << "e: " << keys.e << endl;
    cout << "d: " << keys.d << endl;
    cout << "n: " << keys.n << endl;

    mpz_class data = mprand.get_z_bits(128);
    cout << "* data: " << data << endl;
    mpz_class chiper = rsa(data, keys.e, keys.n);
    cout << "* chiper: " << chiper << endl;
    mpz_class dersa = rsa(chiper, keys.d, keys.n);
    cout << "* decrypted: " << dersa << endl;
}

signed main() {
    testaes();
    testrsa();
}
