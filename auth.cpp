
#include "auth.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <iomanip>
#include <sstream>
#include <vector>
#include <stdexcept>

using namespace std;

static string toHex(
    const unsigned char* data,
    size_t size
) {
    stringstream ss;

    for (size_t i = 0; i < size; i++) {
        ss << hex << setw(2) << setfill('0')
           << static_cast<int>(data[i]);
    }

    return ss.str();
}

static vector<unsigned char> fromHex(const string& value) {
    vector<unsigned char> result;

    for (size_t i = 0; i + 1 < value.size(); i += 2) {
        result.push_back(
            static_cast<unsigned char>(
                stoi(value.substr(i, 2), nullptr, 16)
            )
        );
    }

    return result;
}

string hashPassword(const string& password) {

    unsigned char salt[16];
    unsigned char hash[32];

    if (RAND_bytes(salt, sizeof(salt)) != 1)
        throw runtime_error("Random generation failed");

    if (PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.size()),
        salt,
        sizeof(salt),
        200000,
        EVP_sha256(),
        sizeof(hash),
        hash
    ) != 1) {
        throw runtime_error("Password hashing failed");
    }

    return "200000$" +
        toHex(salt, sizeof(salt)) +
        "$" +
        toHex(hash, sizeof(hash));
}

bool verifyPassword(
    const string& password,
    const string& storedHash
) {
    try {
        size_t first = storedHash.find('$');
        size_t second = storedHash.find('$', first + 1);

        if (first == string::npos ||
            second == string::npos)
            return false;

        int iterations = stoi(storedHash.substr(0, first));

        if (iterations != 200000)
            return false;

        auto salt = fromHex(
            storedHash.substr(first + 1, second - first - 1)
        );

        auto expected = fromHex(
            storedHash.substr(second + 1)
        );

        if (salt.size() != 16 || expected.size() != 32)
            return false;

        unsigned char actual[32];

        if (PKCS5_PBKDF2_HMAC(
            password.c_str(),
            static_cast<int>(password.size()),
            salt.data(),
            static_cast<int>(salt.size()),
            iterations,
            EVP_sha256(),
            sizeof(actual),
            actual
        ) != 1)
            return false;

        return CRYPTO_memcmp(
            actual,
            expected.data(),
            sizeof(actual)
        ) == 0;

    } catch (...) {
        return false;
    }
}

string generateToken() {

    unsigned char bytes[32];

    if (RAND_bytes(bytes, sizeof(bytes)) != 1)
        throw runtime_error("Token generation failed");

    return toHex(bytes, sizeof(bytes));
}
