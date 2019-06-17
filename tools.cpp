#include "tools.h"

namespace tools {

// random

bool _randinited = false;
gmp_randclass _mprand{gmp_randinit_mt};
std::mt19937_64 _randeng;
std::uniform_int_distribution<Byte> _bytedist;

void _initrand() {
    std::random_device rd;
    _mprand.seed(rd());
    _randeng.seed(rd());
    _randinited = true;
}

Byte randbyte() {
    if (!_randinited) _initrand();
    return _bytedist(_randeng);
}

gmp_randclass &getgmprand() {
    if (!_randinited) _initrand();
    return _mprand;
}

// int io

Byte ctoi(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

size_t writempz(Byte *buf, size_t maxlen, mpz_class n) {
    std::string s = n.get_str(16);
    if ((s.size() + 1) / 2 > maxlen)
        return SIZE_MAX;

    auto p = s.begin();
    size_t i = 0;
    if (s.size() & 1) {
        buf[i++] = ctoi(*p);
        ++p;
    }
    while (p != s.end()) {
        buf[i] = ctoi(*p++) << 4;
        buf[i] |= ctoi(*p++);
        ++i;
    }

    return i;
}

}
