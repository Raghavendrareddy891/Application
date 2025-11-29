
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "chat_client.hpp"

void print_usage(const char* prog) {
    std::cout << "Usage:\n"
              << "  " << prog << " register <username> <password>\n"
              << "  " << prog << " login <username> <password>\n"
              << "  " << prog << " send <username> <password> <to> <message>\n"
              << "  " << prog << " listen <username> <password>\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string base_url = "http://127.0.0.1:8000";
    std::string cmd = argv[1];

    try {
        if (cmd == "register") {
            if (argc != 4) {
                print_usage(argv[0]);
                return 1;
            }
            std::string username = argv[2];
            std::string password = argv[3];

            ChatClient client(base_url);
            client.register_user(username, password);
        } else if (cmd == "login") {
            if (argc != 4) {
                print_usage(argv[0]);
                return 1;
            }
            std::string username = argv[2];
            std::string password = argv[3];

            ChatClient client(base_url);
            client.login(username, password);
        } else if (cmd == "send") {
            if (argc < 6) {
                print_usage(argv[0]);
                return 1;
            }
            std::string username = argv[2];
            std::string password = argv[3];
            std::string to = argv[4];
            std::string message;
            for (int i = 5; i < argc; ++i) {
                if (i > 5) message += " ";
                message += argv[i];
            }

            ChatClient client(base_url);
            client.register_user(username, password); // first time ok; next time will 400 if exists
            client.login(username, password);
            client.prepare_session_with(to);
            client.send_message(to, message);
        } else if (cmd == "listen") {
            if (argc != 4) {
                print_usage(argv[0]);
                return 1;
            }
            std::string username = argv[2];
            std::string password = argv[3];

            ChatClient client(base_url);
            client.register_user(username, password);
            client.login(username, password);

            int since_id = 0;
            while (true) {
                since_id = client.poll_messages(since_id);
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        } else {
            print_usage(argv[0]);
            return 1;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
