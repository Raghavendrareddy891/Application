
#include "crypto.hpp"
#include <stdexcept>
#include <cstring>

Crypto::Crypto() {
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to init libsodium");
    }
}

void Crypto::generate_identity() {
    crypto_box_keypair(pk_, sk_);
}

std::string Crypto::get_public_key_b64() const {
    return to_base64(pk_, sizeof(pk_));
}

void Crypto::derive_session_key(const std::string& peer_pub_b64) {
    auto peer_pub = from_base64(peer_pub_b64);
    if (peer_pub.size() != crypto_box_PUBLICKEYBYTES) {
        throw std::runtime_error("Invalid peer public key size");
    }
    unsigned char shared[crypto_scalarmult_BYTES];
    if (crypto_scalarmult(shared, sk_, peer_pub.data()) != 0) {
        throw std::runtime_error("crypto_scalarmult failed");
    }
    std::memcpy(sym_key_, shared, crypto_secretbox_KEYBYTES);
    sym_key_set_ = true;
}

Crypto::CipherData Crypto::encrypt(const std::string& plaintext) {
    if (!sym_key_set_) {
        throw std::runtime_error("Session key not set");
    }

    CipherData out;
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    std::vector<unsigned char> cipher(plaintext.size() + crypto_secretbox_MACBYTES);
    if (crypto_secretbox_easy(cipher.data(),
                              reinterpret_cast<const unsigned char*>(plaintext.data()),
                              plaintext.size(),
                              nonce,
                              sym_key_) != 0) {
        throw std::runtime_error("crypto_secretbox_easy failed");
    }

    out.nonce_b64  = to_base64(nonce, sizeof(nonce));
    out.cipher_b64 = to_base64(cipher.data(), cipher.size());
    return out;
}

std::string Crypto::decrypt(const std::string& nonce_b64,
                            const std::string& cipher_b64) {
    if (!sym_key_set_) {
        throw std::runtime_error("Session key not set");
    }

    auto nonce_vec  = from_base64(nonce_b64);
    auto cipher_vec = from_base64(cipher_b64);
    if (nonce_vec.size() != crypto_secretbox_NONCEBYTES) {
        throw std::runtime_error("Invalid nonce size");
    }

    std::vector<unsigned char> decrypted(cipher_vec.size() - crypto_secretbox_MACBYTES);
    if (crypto_secretbox_open_easy(decrypted.data(),
                                   cipher_vec.data(),
                                   cipher_vec.size(),
                                   nonce_vec.data(),
                                   sym_key_) != 0) {
        throw std::runtime_error("Decryption failed (forged or wrong key)");
    }

    return std::string(decrypted.begin(), decrypted.end());
}

std::string Crypto::to_base64(const unsigned char* data, size_t len) {
    size_t out_len = sodium_base64_encoded_len(len, sodium_base64_VARIANT_ORIGINAL);
    std::string out(out_len, '\0');
    sodium_bin2base64(out.data(), out.size(),
                      data, len,
                      sodium_base64_VARIANT_ORIGINAL);
    out.resize(std::strlen(out.c_str()));
    return out;
}

std::vector<unsigned char> Crypto::from_base64(const std::string& b64) {
    std::vector<unsigned char> out(b64.size());
    size_t out_len;
    if (sodium_base642bin(out.data(), out.size(),
                          b64.c_str(), b64.size(),
                          nullptr, &out_len, nullptr,
                          sodium_base64_VARIANT_ORIGINAL) != 0) {
        throw std::runtime_error("b64 decode failed");
    }
    out.resize(out_len);
    return out;
}
