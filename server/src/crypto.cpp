#include "../include/crypto.hpp"

std::vector<unsigned char> Crypto::generateSalt() {
    std::vector<unsigned char> salt(SALT_LENGTH);
    if (!RAND_bytes(salt.data(), SALT_LENGTH)) {
        throw std::runtime_error("Failed to generate random salt");
    }
    return salt;
}

std::vector<unsigned char> Crypto::pbkdf2(const std::string& password, const std::vector<unsigned char>& salt) {
    std::vector<unsigned char> hash(HASH_LENGTH);
    if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.length(), salt.data(), salt.size(), ITERATIONS, EVP_sha256(), HASH_LENGTH, hash.data())) {
        throw std::runtime_error("Failed to hash password using PBKDF2");
    }
    return hash;
}

std::string Crypto::toHex(const std::vector<unsigned char>& data) {
    std::ostringstream oss;
    for (unsigned char byte : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

std::vector<unsigned char> Crypto::toBinary(const std::string& data) {
    std::vector<unsigned char> result;
    for (size_t i = 0; i < data.length(); i += 2) {
        std::string byteString = data.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
        result.push_back(byte);
    }
    return result;
}

std::string Crypto::encodeResult(const std::vector<unsigned char>& salt, const std::vector<unsigned char>& hash) {
    std::ostringstream oss;

    oss << "$pbkdf2$" << ITERATIONS << "$";

    oss << toHex(salt) << "$";

    oss << toHex(hash);

    return oss.str();
}

std::pair<std::string, std::string> Crypto::decodeResult(const std::string& storedHash) {
    // storedHash: $pbkdf2$100000$<salt>$<hash>
    size_t firstDollar = storedHash.find('$');
    size_t secondDollar = storedHash.find('$', firstDollar + 1);
    size_t thirdDollar = storedHash.find('$', secondDollar + 1);
    size_t fourthDollar = storedHash.find('$', thirdDollar + 1);  

    if (firstDollar == std::string::npos || secondDollar == std::string::npos || thirdDollar == std::string::npos || fourthDollar == std::string::npos) {
        throw std::invalid_argument("Invalid stored hash format");
    }

    std::string saltHex = storedHash.substr(thirdDollar + 1, fourthDollar - thirdDollar - 1);
    std::string hashHex = storedHash.substr(fourthDollar + 1);

    return {saltHex, hashHex};
}
std::string Crypto::hashPassword(const std::string& password) {
    std::vector<unsigned char> salt = generateSalt();
    std::vector<unsigned char> hash = pbkdf2(password, salt);

    return encodeResult(salt, hash);
}

bool Crypto::verifyPasswords(const std::string& storedHash, const std::string& providedPassword) {
    // try {
    //     auto [saltHex, storedHashHex] = decodeResult(storedHash);

    //     // std::vector<unsigned char> salt = toBinary(saltHex);
    //     // std::vector<unsigned char> storedHash = toBinary(storedHashHex);

    //     std::cout<<"Stored Hash: "<<storedHashHex<<"\n";
    //     std::cout<<"Salt hex: "<< saltHex;

    //     std::vector<unsigned char> providedHash = pbkdf2(providedPassword, salt);

    //     if (providedHash == storedHash) {
    //         std::cout << "Correct password\n";
    //         return true;
    //     } else {
    //         std::cout << "Wrong password\n";
    //         return false;
    //     }
    // } catch (const std::exception& e) {
    //     std::cerr << "Error: " << e.what();
    //     return false;
    // }

    auto [storedSaltHex, storedHashHex] = decodeResult(storedHash);

    std::vector<unsigned char> saltBinary = toBinary(storedSaltHex);
    std::vector<unsigned char> derivedHashBinary = pbkdf2(providedPassword, saltBinary);

    std::string derivedHashHex = toHex(derivedHashBinary);

    std::cout << "StoredSaltHex: " << storedSaltHex << "\n";
    std::cout << "StoredHashHex: " << storedHashHex << "\n";
    std::cout << "DerivedHashHex: " << derivedHashHex << "\n";

    if (storedHashHex == derivedHashHex) {
        std::cout << "correct\n";
        return true;
    } else {
        std::cout << "wrong\n";
        return false;
    }
}
