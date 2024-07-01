//
// Created by amir on 01.07.24.
//

#include "../include/database_client.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>

std::vector<std::vector<std::string>> DatabaseClient::execute_query(const std::string& query) {
    try {
        connect_to_server();
        send_query(query);
        return receive_results();
    } catch (const std::exception& e) {
        std::cerr << "Error executing query: " << e.what() << std::endl;
        return {};
    }
}

void DatabaseClient::connect_to_server() {
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        throw std::runtime_error("Invalid address / Address not supported");
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        throw std::runtime_error("Connection failed: " + std::string(strerror(errno)));
    }
}

void DatabaseClient::send_query(const std::string& query) {
    int total_sent = 0;
    int remaining = query.length();
    const char* ptr = query.c_str();

    while (total_sent < query.length()) {
        int sent = send(sock, ptr + total_sent, remaining, 0);
        if (sent < 0) {
            throw std::runtime_error("Error sending query: " + std::string(strerror(errno)));
        }
        total_sent += sent;
        remaining -= sent;
    }
}

std::vector<std::vector<std::string>> DatabaseClient::receive_results() {
    std::string response;
    char buffer[1024];
    int bytes_received;

    while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        response += buffer;

        if (bytes_received < sizeof(buffer) - 1) {
            break;
        }
    }

    if (bytes_received < 0) {
        throw std::runtime_error("Error receiving results: " + std::string(strerror(errno)));
    }

    if (response.substr(0, 6) == "Error:") {
        throw std::runtime_error(response.substr(7));
    }

    return deserialize_results(response);
}

DatabaseClient::DatabaseClient(const std::string& ip, int port) : server_ip(ip), server_port(port) {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Failed to create socket");
    }
}

DatabaseClient::~DatabaseClient() {
    close(sock);
}

std::vector<std::vector<std::string>> DatabaseClient::deserialize_results(const std::string& serialized) {
    std::vector<std::vector<std::string>> results;
    std::istringstream iss(serialized);
    std::string line;

    while (std::getline(iss, line)) {
        std::vector<std::string> row;
        std::istringstream line_stream(line);
        std::string value;

        while (std::getline(line_stream, value, '|')) {
            if (!value.empty()) {
                row.push_back(value);
            }
        }

        if (!row.empty()) {
            results.push_back(row);
        }
    }

    return results;
}