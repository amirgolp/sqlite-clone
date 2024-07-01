//
// Created by amir on 01.07.24.
//
#include "query_parser.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#pragma once
#ifndef SQLITE_DATABASE_H
#define SQLITE_DATABASE_H

class Table {
public:
    Table(const std::string& name, const std::vector<std::string>& columns);
    void insert(const std::vector<std::string>& values);
    std::vector<std::string> select(int key) const;
    void update(int key, const std::vector<std::string>& values);
    void remove(int key);
    int get_row_count() const;
    const std::vector<std::string>& get_columns() const;
    int get_column_index(const std::string& column_name) const;

private:
    std::string name;
    std::vector<std::string> columns;
    std::unordered_map<int, std::vector<std::string>> data;
};

class Database {
public:
    std::vector<std::vector<std::string>> execute_query(const std::string& query);

private:
    std::unordered_map<std::string, std::shared_ptr<Table>> tables;
    std::vector<std::vector<std::string>> execute_select(const std::string& table_name, const std::vector<std::string>& columns, const std::string& condition);
    void execute_insert(const std::string& table_name, const std::vector<std::string>& values);
    void execute_update(const std::string& table_name,
                        const std::vector<std::string>& columns,
                        const std::vector<std::string>& values,
                        const std::string& condition);
    void execute_delete(const std::string& table_name, const std::string& condition);
    void create_table(const std::basic_string<char> &name, const std::vector<std::basic_string<char>> &columns);
};

#endif
