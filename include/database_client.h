//
// Created by amir on 01.07.24.
//
#pragma once
#include <string>
#include <vector>
#ifndef SQLITE_DATABASE_CLIENT_H
#define SQLITE_DATABASE_CLIENT_H


class DatabaseClient {
public:
    DatabaseClient(const std::string& ip, int port);
    ~DatabaseClient();
    std::vector<std::vector<std::string>> execute_query(const std::string& query);

private:
    int sock;
    std::string server_ip;
    int server_port;

    void connect_to_server();
    void send_query(const std::string& query);
    std::vector<std::vector<std::string>> receive_results();
    std::vector<std::vector<std::string>> deserialize_results(const std::string& serialized);
};
#endif //SQLITE_DATABASE_CLIENT_H
