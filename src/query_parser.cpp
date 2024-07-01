//
// Created by amir on 01.07.24.
//

#include "../include/query_parser.h"
#include <sstream>
#include <algorithm>
#include <unordered_set>

std::vector<QueryParser::Token> QueryParser::tokenize(const std::string& query) {
    std::vector<Token> tokens;
    std::istringstream iss(query);
    std::string word;

    while (iss >> word) {
        if (is_keyword(word)) {
            tokens.emplace_back(Token::KEYWORD, word);
        } else if (is_operator(word)) {
            tokens.emplace_back(Token::OPERATOR, word);
        } else if (word[0] == '\'' || word[0] == '"') {
            std::string value;
            char quote = word[0];
            value += word.substr(1);
            if (word.back() != quote) {
                std::string more;
                while (iss >> more) {
                    value += " " + more;
                    if (more.back() == quote) break;
                }
            }
            value = value.substr(0, value.length() - 1);
            tokens.emplace_back(Token::VALUE, value);
        } else {
            tokens.emplace_back(Token::IDENTIFIER, word);
        }
    }
    tokens.emplace_back(Token::END, "");
    return tokens;
}

void QueryParser::parse(const std::vector<Token>& tokens, std::string& command, std::string& table_name,
                        std::vector<std::string>& columns, std::vector<std::string>& values, std::string& condition) {
    if (tokens.empty() || tokens[0].type != Token::KEYWORD) {
        throw QueryParseError("Query must start with a keyword");
    }

    command = tokens[0].value;
    size_t i = 1;

    if (command == "SELECT") {
        while (i < tokens.size() && tokens[i].type != Token::KEYWORD) {
            if (tokens[i].type == Token::IDENTIFIER) {
                columns.push_back(tokens[i].value);
            }
            ++i;
        }
        if (i >= tokens.size() || tokens[i].value != "FROM") {
            throw QueryParseError("SELECT query must have a FROM clause");
        }
        ++i;
        if (i >= tokens.size() || tokens[i].type != Token::IDENTIFIER) {
            throw QueryParseError("Table name expected after FROM");
        }
        table_name = tokens[i].value;
        ++i;
        if (i < tokens.size() && tokens[i].value == "WHERE") {
            ++i;
            while (i < tokens.size() && tokens[i].type != Token::END) {
                condition += tokens[i].value + " ";
                ++i;
            }
        }
    } else if (command == "INSERT") {
        if (i >= tokens.size() || tokens[i].value != "INTO") {
            throw QueryParseError("INSERT query must have an INTO clause");
        }
        ++i;
        if (i >= tokens.size() || tokens[i].type != Token::IDENTIFIER) {
            throw QueryParseError("Table name expected after INTO");
        }
        table_name = tokens[i].value;
        ++i;
        if (i >= tokens.size() || tokens[i].value != "VALUES") {
            throw QueryParseError("INSERT query must have a VALUES clause");
        }
        ++i;
        while (i < tokens.size() && tokens[i].type != Token::END) {
            if (tokens[i].type == Token::VALUE) {
                values.push_back(tokens[i].value);
            }
            ++i;
        }
    } else if (command == "UPDATE") {
        if (i >= tokens.size() || tokens[i].type != Token::IDENTIFIER) {
            throw QueryParseError("Table name expected after UPDATE");
        }
        table_name = tokens[i].value;
        ++i;
        if (i >= tokens.size() || tokens[i].value != "SET") {
            throw QueryParseError("UPDATE query must have a SET clause");
        }
        ++i;
        while (i < tokens.size() && tokens[i].value != "WHERE") {
            if (tokens[i].type == Token::IDENTIFIER) {
                columns.push_back(tokens[i].value);
            } else if (tokens[i].type == Token::VALUE) {
                values.push_back(tokens[i].value);
            }
            ++i;
        }
        if (i < tokens.size() && tokens[i].value == "WHERE") {
            ++i;
            while (i < tokens.size() && tokens[i].type != Token::END) {
                condition += tokens[i].value + " ";
                ++i;
            }
        }
    } else if (command == "DELETE") {
        if (i >= tokens.size() || tokens[i].value != "FROM") {
            throw QueryParseError("DELETE query must have a FROM clause");
        }
        ++i;
        if (i >= tokens.size() || tokens[i].type != Token::IDENTIFIER) {
            throw QueryParseError("Table name expected after FROM");
        }
        table_name = tokens[i].value;
        ++i;
        if (i < tokens.size() && tokens[i].value == "WHERE") {
            ++i;
            while (i < tokens.size() && tokens[i].type != Token::END) {
                condition += tokens[i].value + " ";
                ++i;
            }
        }
    } else {
        throw QueryParseError("Unknown command: " + command);
    }
}

bool QueryParser::is_keyword(const std::string& word) {
    static const std::unordered_set<std::string> keywords = {
            "SELECT", "INSERT", "UPDATE", "DELETE", "FROM", "WHERE", "VALUES", "SET", "INTO"
    };
    return keywords.find(word) != keywords.end();
}

bool QueryParser::is_operator(const std::string& word) {
    static const std::unordered_set<std::string> operators = {
            "=", "<", ">", "<=", ">=", "!="
    };
    return operators.find(word) != operators.end();
}