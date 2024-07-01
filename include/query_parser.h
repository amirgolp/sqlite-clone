//
// Created by amir on 01.07.24.
//
#include <string>
#include <vector>
#include <stdexcept>

#ifndef SQLITE_QUERY_PARSER_H
#define SQLITE_QUERY_PARSER_H
#pragma once


class QueryParser {
public:
    struct Token {
        enum Type { KEYWORD, IDENTIFIER, VALUE, OPERATOR, END };
        Type type;
        std::string value;
        Token(Type t, const std::string& v) : type(t), value(v) {}
    };

    static std::vector<Token> tokenize(const std::string& query);
    static void parse(const std::vector<Token>& tokens, std::string& command, std::string& table_name,
                      std::vector<std::string>& columns, std::vector<std::string>& values, std::string& condition);

private:
    static bool is_keyword(const std::string& word);
    static bool is_operator(const std::string& word);
};

class QueryParseError : public std::runtime_error {
public:
    QueryParseError(const std::string& message) : std::runtime_error(message) {}
};
#endif
