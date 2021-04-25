#include <gtest/gtest.h>

#include <random>
using namespace std;

#include "../src/encrypt/rsa.hpp"


class RsaTester: public testing::Test
{
protected:
    inline static gmp_randclass mprand{gmp_randinit_mt};

    static void SetUpTestCase()
    {
        random_device rd;
        mprand.seed(rd());
    }

    static void TearDownTestCase() {}
};


TEST_F(RsaTester, 1)
{
    auto keys = rsa_keygen(mprand);
    mpz_class data = mprand.get_z_bits(128);
    mpz_class chiper = rsa(data, keys.e, keys.n);
    ASSERT_NE(data, chiper);
    mpz_class decrypted = rsa(chiper, keys.d, keys.n);
    ASSERT_EQ(data, decrypted);
}
