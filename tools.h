#ifndef CAST_H
#define CAST_H

#include "pch.hpp"

namespace tools {

// absolutely random

Byte randbyte();

gmp_randclass &getgmprand();

// int io (big-ending)

template <typename NUM>
NUM readint(const Byte *p, size_t len) {
    NUM res = 0;
    for (size_t i = 0; i < len; i++) {
        res <<= 8;
        res += p[i];
    }
    return res;
}

// write exactly sizeof(n) bytes
template <typename INT_T>
void writeint(Byte *buf, INT_T n) {
    for (int i = sizeof(n) - 1; i >= 0; i--) {
        buf[i] = n & 0xff;
        n >>= 8;
    }
}

// return writed bytes, or SIZE_MAX if maxlen less than the length of n
size_t writempz(Byte *buf, size_t maxlen, mpz_class n);

}

#endif // CAST_H
