#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <iomanip>
#include <iostream>
#include <vector>

class Crypto {
   private:
    const int SALT_LENGTH = 32;
    const int HASH_LENGTH = 32;
    const int ITERATIONS = 100000;

    std::vector<unsigned char> generateSalt();
    std::vector<unsigned char> pbkdf2(const std::string& password, const std::vector<unsigned char>& salt);
    std::string toHex(const std::vector<unsigned char>& data);
    std::vector<unsigned char> toBinary(const std::string& data);
    std::string encodeResult(const std::vector<unsigned char>& salt, const std::vector<unsigned char>& hash);
    std::pair<std::string, std::string> decodeResult(const std::string& storedHash);  // <salt, hash>

   public:
    std::string hashPassword(const std::string& password);
    bool verifyPasswords(const std::string& storedHash, const std::string& plainPassword);
};

#endif