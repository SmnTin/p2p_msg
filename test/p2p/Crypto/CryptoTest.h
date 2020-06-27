#ifndef P2P_MSG_CRYPTOTEST_H
#define P2P_MSG_CRYPTOTEST_H

#include "gtest/gtest.h"
#include "p2p/Crypto/ECC.h"
#include "p2p/Crypto/AES.h"

using namespace p2p::Crypto;

namespace {
    TEST(ECC, SharedKey) {
        ECC::PublicKey pub1, pub2;
        ECC::PrivateKey pri1, pri2;
        ECC::generateKeys(pub1, pri1);
        ECC::generateKeys(pub2, pri2);

        ECC::SharedKey sh1, sh2;
        ECC::generateSharedKey(pub2, pri1, sh1);
        ECC::generateSharedKey(pub1, pri2, sh2);
        EXPECT_EQ(sh1, sh2);
    }

    TEST(AES, ECB_EncryptDecrypt) {
        ECC::PublicKey pub;
        ECC::PrivateKey pri;
        ECC::generateKeys(pub, pri);
        ECC::SharedKey sh1;
        ECC::generateSharedKey(pub, pri, sh1);

        AES::SharedKey sh = sh1;
        std::string s = "PIZDA";
        std::size_t len;
        auto out = AES::encryptECB(s.data(), s.length(), sh, len);
        auto out2 = AES::decryptECB(out, len, sh, len);
        std::string s2(s.length(), ' ');
        memcpy(s2.data(), out2, s.length());
        EXPECT_EQ(s, s2);
        delete[] out;
        delete[] out2;
    }

    TEST(AES, ECB_EncryptDecryptSafe) {
        ECC::SharedKey sh1 = ECC::generateSharedKey(ECC::generateKeys());

        AES::SharedKey sh = sh1;
        std::string s = "fown40eof3nrnf394f39iu4fn3unr0v93u";
        std::size_t len;
        auto out = AES::encryptECBSafe(s.data(), s.length(), sh, len);
        auto out2 = AES::decryptECBSafe(out.get(), len, sh, len);
        std::string s2(s.length(), ' ');
        memcpy(s2.data(), out2.get(), s.length());
        EXPECT_EQ(s, s2);
    }

    TEST(AES, ECB_EncryptDecryptString) {
        ECC::SharedKey sh1 = ECC::generateSharedKey(ECC::generateKeys());

        AES::SharedKey sh = sh1;
        std::string s = "d2u4fh93w4euf3w4fu3i4bf";
        auto out = AES::encryptECB(s, sh);
        auto out2 = AES::decryptECB(out, sh);
        std::string s2(s.size(), ' ');
        memcpy(s2.data(), out2.data(), s.size());
        EXPECT_EQ(s, s2);
    }
}

#endif //P2P_MSG_CRYPTOTEST_H
