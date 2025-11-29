
#include "chat_client.hpp"
#include <cpr/cpr.h>
#include <iostream>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ChatClient::ChatClient(const std::string& base_url)
    : base_url_(base_url) {
    crypto_.generate_identity();
}

std::string ChatClient::http_get(const std::string& path) {
    cpr::Header headers;
    if (!token_.empty()) {
        headers["Authorization"] = "Bearer " + token_;
    }
    auto r = cpr::Get(cpr::Url{base_url_ + path}, headers);
    if (r.status_code >= 400) {
        throw std::runtime_error("GET failed: " + std::to_string(r.status_code) + " " + r.text);
    }
    return r.text;
}

std::string ChatClient::http_post(const std::string& path, const std::string& body) {
    cpr::Header headers{{"Content-Type", "application/json"}};
    if (!token_.empty()) {
        headers["Authorization"] = "Bearer " + token_;
    }
    auto r = cpr::Post(cpr::Url{base_url_ + path},
                       headers,
                       cpr::Body{body});
    if (r.status_code >= 400) {
        throw std::runtime_error("POST failed: " + std::to_string(r.status_code) + " " + r.text);
    }
    return r.text;
}

void ChatClient::register_user(const std::string& username,
                               const std::string& password) {
    json j{
        {"username", username},
        {"password", password},
        {"identity_public_key", crypto_.get_public_key_b64()}
    };
    auto resp = http_post("/register", j.dump());
    std::cout << "Register response: " << resp << std::endl;
}

void ChatClient::login(const std::string& username,
                       const std::string& password) {
    json j{
        {"username", username},
        {"password", password}
    };
    auto resp_str = http_post("/login", j.dump());
    auto resp = json::parse(resp_str);
    token_ = resp.at("token").get<std::string>();
    std::cout << "Logged in. Token acquired.\n";
}

void ChatClient::prepare_session_with(const std::string& peer_username) {
    auto resp_str = http_get("/users/" + peer_username + "/public-key");
    auto resp = json::parse(resp_str);
    std::string peer_pub_b64 = resp.at("identity_public_key").get<std::string>();
    crypto_.derive_session_key(peer_pub_b64);
    std::cout << "Session key derived with " << peer_username << std::endl;
}

void ChatClient::send_message(const std::string& to,
                              const std::string& plaintext) {
    auto c = crypto_.encrypt(plaintext);
    json j{
        {"to", to},
        {"ciphertext", c.cipher_b64},
        {"nonce", c.nonce_b64}
    };
    auto resp = http_post("/messages", j.dump());
    std::cout << "Send response: " << resp << std::endl;
}

int ChatClient::poll_messages(int since_id) {
    auto resp_str = http_get("/messages?since_id=" + std::to_string(since_id));
    auto resp = json::parse(resp_str);
    int max_id = since_id;
    for (auto& m : resp["messages"]) {
        int id = m.at("id").get<int>();
        std::string from = m.at("from_").get<std::string>();
        std::string nonce_b64 = m.at("nonce").get<std::string>();
        std::string cipher_b64 = m.at("ciphertext").get<std::string>();
        std::string plaintext = crypto_.decrypt(nonce_b64, cipher_b64);
        std::cout << "[" << id << "] " << from << ": " << plaintext << std::endl;
        if (id > max_id) max_id = id;
    }
    return max_id;
}
