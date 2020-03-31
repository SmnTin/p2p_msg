#include "p2p/Crypto/AES.h"
#include "p2p/Crypto/ECC.h"

#include <iostream>
#include <cassert>
#include <cstring>

using namespace p2p::Crypto;

int main() {
    ECC::PublicKey pub1, pub2;
    ECC::PrivateKey pri1, pri2;
    ECC::generateKeys(pub1, pri1);
    ECC::generateKeys(pub2, pri2);

    ECC::SharedKey sh1, sh2;
    ECC::generateSharedKey(pub2, pri1, sh1);
    ECC::generateSharedKey(pub1, pri2, sh2);
    assert(sh1 == sh2);

    AES::SharedKey sh = sh1;
    std::string s = "PIZDA";
    std::size_t len;
    auto out = AES::encryptECB(s.data(), s.length(), sh, len);
    auto out2 = AES::decryptECB(out, len, sh, len);
    std::string s2(len, ' ');
    memcpy(s2.data(), out2, len);
    std::cout << s2 << "\n" << (int)s2[13] << "\n";
}