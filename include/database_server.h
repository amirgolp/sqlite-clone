//
// Created by amir on 01.07.24.
//
#include "database.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <queue>
#include <condition_variable>
#ifndef SQLITE_DATABASE_SERVER_H
#define SQLITE_DATABASE_SERVER_H
#pragma once


class DatabaseServer {
public:
    DatabaseServer(int port);
    ~DatabaseServer();
    void run();
    void stop();

private:
    Database db;
    int server_fd;
    std::atomic<bool> running;
    std::vector<std::thread> worker_threads;
    std::queue<int> client_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    const int num_workers = 4;

    void setup_server(int port);
    int accept_connection();
    void start_workers();
    void worker_function();
    void handle_client(int client_socket);
    void send_response(int client_socket, const std::basic_string<char> &response);
    std::string serialize_results(const std::vector<std::vector<std::string>>& results);
};
#endif //SQLITE_DATABASE_SERVER_H
