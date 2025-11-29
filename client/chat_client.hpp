
#ifndef CHAT_CLIENT_HPP
#define CHAT_CLIENT_HPP

#include <string>
#include "crypto.hpp"

class ChatClient {
public:
    explicit ChatClient(const std::string& base_url);

    void register_user(const std::string& username,
                       const std::string& password);

    void login(const std::string& username,
               const std::string& password);

    void prepare_session_with(const std::string& peer_username);

    void send_message(const std::string& to,
                      const std::string& plaintext);

    int poll_messages(int since_id);

private:
    std::string base_url_;
    std::string token_;
    Crypto crypto_;

    std::string http_get(const std::string& path);
    std::string http_post(const std::string& path, const std::string& json_body);
};

#endif
