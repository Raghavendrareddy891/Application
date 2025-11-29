
#ifndef CRYPTO_HPP
#define CRYPTO_HPP

#include <sodium.h>
#include <string>
#include <vector>

class Crypto {
public:
    Crypto();

    // Identity keypair (long-term)
    void generate_identity();
    std::string get_public_key_b64() const;

    // Derive symmetric session key from peer's public key (base64)
    void derive_session_key(const std::string& peer_pub_b64);

    struct CipherData {
        std::string nonce_b64;
        std::string cipher_b64;
    };

    CipherData encrypt(const std::string& plaintext);
    std::string decrypt(const std::string& nonce_b64,
                        const std::string& cipher_b64);

private:
    unsigned char sk_[crypto_box_SECRETKEYBYTES]{};
    unsigned char pk_[crypto_box_PUBLICKEYBYTES]{};
    unsigned char sym_key_[crypto_secretbox_KEYBYTES]{};
    bool sym_key_set_ = false;

    static std::string to_base64(const unsigned char* data, size_t len);
    static std::vector<unsigned char> from_base64(const std::string& b64);
};

#endif
