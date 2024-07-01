//
// Created by amir on 01.07.24.
//

#include "../include/database_server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

void DatabaseServer::run() {
    while (running) {
        try {
            int client_socket = accept_connection();
            if (client_socket >= 0) {
                std::lock_guard<std::mutex> lock(queue_mutex);
                client_queue.push(client_socket);
                queue_cv.notify_one();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error in main server loop: " << e.what() << std::endl;
            // Optionally, add a short sleep to prevent tight error loops
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void DatabaseServer::worker_function() {
    while (running) {
        int client_socket = -1;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cv.wait(lock, [this] { return !client_queue.empty() || !running; });
            if (!running) return;
            client_socket = client_queue.front();
            client_queue.pop();
        }

        try {
            handle_client(client_socket);
        } catch (const std::exception& e) {
            std::cerr << "Error handling client: " << e.what() << std::endl;
        }

        close(client_socket);
    }
}

void DatabaseServer::handle_client(int client_socket) {
    char buffer[1024] = {0};
    int valread = read(client_socket, buffer, 1024);
    if (valread < 0) {
        throw std::runtime_error("Error reading from socket");
    }
    std::string query(buffer);

    try {
        auto results = db.execute_query(query);
        std::string response = serialize_results(results);
        send_response(client_socket, response);
    } catch (const std::exception& e) {
        std::string error_msg = "Error: " + std::string(e.what());
        send_response(client_socket, error_msg);
    }
}

void DatabaseServer::send_response(int client_socket, const std::string& response) {
    int total_sent = 0;
    int remaining = response.length();
    const char* ptr = response.c_str();

    while (total_sent < response.length()) {
        int sent = send(client_socket, ptr + total_sent, remaining, 0);
        if (sent < 0) {
            throw std::runtime_error("Error sending response: " + std::string(strerror(errno)));
        }
        total_sent += sent;
        remaining -= sent;
    }
}



DatabaseServer::DatabaseServer(int port) : running(true) {
    setup_server(port);
    start_workers();
}

DatabaseServer::~DatabaseServer() {
    stop();
}

void DatabaseServer::stop() {
    running = false;
    close(server_fd);
    queue_cv.notify_all();
    for (auto& thread : worker_threads) {
        thread.join();
    }
}

void DatabaseServer::setup_server(int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        throw std::runtime_error("Failed to set socket options");
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Failed to bind to port");
    }

    if (listen(server_fd, 3) < 0) {
        throw std::runtime_error("Failed to listen");
    }
}

int DatabaseServer::accept_connection() {
    sockaddr_in address;
    int addrlen = sizeof(address);
    int client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    return client_socket;
}

void DatabaseServer::start_workers() {
    for (int i = 0; i < num_workers; ++i) {
        worker_threads.emplace_back(&DatabaseServer::worker_function, this);
    }
}

std::string DatabaseServer::serialize_results(const std::vector<std::vector<std::string>>& results) {
    std::string serialized;
    for (const auto& row : results) {
        for (const auto& value : row) {
            serialized += value + "|";
        }
        serialized += "\n";
    }
    return serialized;
}