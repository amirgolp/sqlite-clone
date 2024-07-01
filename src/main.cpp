#include "../include/database_server.h"
#include "../include/database_client.h"
#include <iostream>
#include <string>

void run_server(int port) {
    try {
        DatabaseServer server(port);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
}

void run_client(const std::string& ip, int port) {
    try {
        DatabaseClient client(ip, port);

        while (true) {
            std::string query;
            std::cout << "Enter query (or 'quit' to exit): ";
            std::getline(std::cin, query);

            if (query == "quit") {
                break;
            }

            try {
                auto results = client.execute_query(query);
                for (const auto& row : results) {
                    for (const auto& value : row) {
                        std::cout << value << "\t";
                    }
                    std::cout << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Query error: " << e.what() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [server|client] [port] [ip]" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    int port = 8080;  // Default port

    if (argc >= 3) {
        port = std::stoi(argv[2]);
    }

    if (mode == "server") {
        run_server(port);
    } else if (mode == "client") {
        std::string ip = "127.0.0.1";  // Default IP
        if (argc >= 4) {
            ip = argv[3];
        }
        run_client(ip, port);
    } else {
        std::cerr << "Invalid mode. Use 'server' or 'client'." << std::endl;
        return 1;
    }

    return 0;
}