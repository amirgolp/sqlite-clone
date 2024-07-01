//
// Created by amir on 01.07.24.
//
#include "../include/database.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <sstream>

std::vector<std::vector<std::string>> Database::execute_query(const std::string& query) {
    try {
        auto tokens = QueryParser::tokenize(query);
        std::string command, table_name, condition;
        std::vector<std::string> columns, values;
        QueryParser::parse(tokens, command, table_name, columns, values, condition);

        if (command == "SELECT") {
            return execute_select(table_name, columns, condition);
        } else if (command == "INSERT") {
            execute_insert(table_name, values);
        } else if (command == "UPDATE") {
            execute_update(table_name, columns, values, condition);
        } else if (command == "DELETE") {
            execute_delete(table_name, condition);
        } else {
            throw std::runtime_error("Unknown command: " + command);
        }
    } catch (const QueryParseError& e) {
        throw std::runtime_error("Query parse error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Error executing query: " + std::string(e.what()));
    }

    return {};
}


Table::Table(const std::string& name, const std::vector<std::string>& columns)
        : name(name), columns(columns) {}

void Table::insert(const std::vector<std::string>& values) {
    if (values.size() != columns.size()) {
        throw std::runtime_error("Number of values doesn't match number of columns");
    }
    int key = std::stoi(values[0]);
    data[key] = values;
}

std::vector<std::string> Table::select(int key) const {
    auto it = data.find(key);
    return it != data.end() ? it->second : std::vector<std::string>();
}

void Table::update(int key, const std::vector<std::string>& values) {
    if (data.find(key) != data.end()) {
        data[key] = values;
    }
}

void Table::remove(int key) {
    data.erase(key);
}

int Table::get_row_count() const {
    return data.size();
}

const std::vector<std::string>& Table::get_columns() const {
    return columns;
}

int Table::get_column_index(const std::string& column_name) const {
    auto it = std::find(columns.begin(), columns.end(), column_name);
    return it != columns.end() ? std::distance(columns.begin(), it) : -1;
}

void Database::create_table(const std::string& name, const std::vector<std::string>& columns) {
    if (tables.find(name) != tables.end()) {
        throw std::runtime_error("Table already exists: " + name);
    }
    tables[name] = std::make_shared<Table>(name, columns);
    std::cout << "Table created: " << name << std::endl;
}

std::vector<std::vector<std::string>> Database::execute_select(const std::string& table_name, const std::vector<std::string>& columns, const std::string& condition) {
    auto it = tables.find(table_name);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found: " + table_name);
    }

    std::vector<std::vector<std::string>> results;
    auto& table = it->second;

    for (int i = 0; i < table->get_row_count(); ++i) {
        auto row = table->select(i);
        if (condition.empty() || row[0] == condition) {
            results.push_back(row);
        }
    }

    return results;
}

void Database::initialize_database() {
    try {
        create_table("users", {"id", "name", "age"});
        create_table("products", {"id", "name", "price"});

        // You can add more tables here as needed
    } catch (const std::exception& e) {
        std::cerr << "Error initializing database: " << e.what() << std::endl;
    }
}

void Database::execute_insert(const std::string& table_name, const std::vector<std::string>& values) {
    auto it = tables.find(table_name);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found: " + table_name);
    }

    it->second->insert(values);
}

void Database::execute_update(const std::string& table_name,
                              const std::vector<std::string>& columns,
                              const std::vector<std::string>& values,
                              const std::string& condition) {
    // Check if the table exists
    auto it = tables.find(table_name);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found: " + table_name);
    }
    auto& table = it->second;

    // Check if the number of columns matches the number of values
    if (columns.size() != values.size()) {
        throw std::runtime_error("Number of columns doesn't match number of values");
    }

    // Prepare a vector to hold the column indices
    std::vector<int> col_indices;
    for (const auto& col : columns) {
        int index = table->get_column_index(col);
        if (index == -1) {
            throw std::runtime_error("Column not found: " + col);
        }
        col_indices.push_back(index);
    }

    // Parse the condition
    std::string condition_column;
    std::string condition_operator;
    std::string condition_value;
    if (!condition.empty()) {
        std::istringstream iss(condition);
        iss >> condition_column >> condition_operator >> condition_value;

        // Remove quotes from condition value if present
        if (condition_value.front() == '\'' && condition_value.back() == '\'') {
            condition_value = condition_value.substr(1, condition_value.length() - 2);
        }
    }

    int condition_col_index = -1;
    if (!condition_column.empty()) {
        condition_col_index = table->get_column_index(condition_column);
        if (condition_col_index == -1) {
            throw std::runtime_error("Condition column not found: " + condition_column);
        }
    }

    // Update the rows
    int updated_count = 0;
    for (int row = 0; row < table->get_row_count(); ++row) {
        auto row_data = table->select(row);

        // Check if this row satisfies the condition
        bool satisfies_condition = condition.empty();
        if (!satisfies_condition) {
            const std::string& cell_value = row_data[condition_col_index];
            if (condition_operator == "=") {
                satisfies_condition = (cell_value == condition_value);
            } else if (condition_operator == ">") {
                satisfies_condition = (std::stod(cell_value) > std::stod(condition_value));
            } else if (condition_operator == "<") {
                satisfies_condition = (std::stod(cell_value) < std::stod(condition_value));
            } else if (condition_operator == ">=") {
                satisfies_condition = (std::stod(cell_value) >= std::stod(condition_value));
            } else if (condition_operator == "<=") {
                satisfies_condition = (std::stod(cell_value) <= std::stod(condition_value));
            } else if (condition_operator == "!=") {
                satisfies_condition = (cell_value != condition_value);
            } else {
                throw std::runtime_error("Unknown operator in condition: " + condition_operator);
            }
        }

        if (satisfies_condition) {
            // Update the values for this row
            for (size_t i = 0; i < col_indices.size(); ++i) {
                row_data[col_indices[i]] = values[i];
            }
            table->update(std::stoi(row_data[0]), row_data);
            updated_count++;
        }
    }

    std::cout << "Updated " << updated_count << " row(s)" << std::endl;
}

void Database::execute_delete(const std::string& table_name, const std::string& condition) {
    auto it = tables.find(table_name);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found: " + table_name);
    }

    auto& table = it->second;
    for (int i = table->get_row_count() - 1; i >= 0; --i) {
        auto row = table->select(i);
        if (condition.empty() || row[0] == condition) {
            table->remove(std::stoi(row[0]));
        }
    }
}