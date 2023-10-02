#pragma once

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

enum class TokenType {
    EXIT,
    INT_LITERAL,
    SEMICOLON,
    OPEN_PAREN,
    CLOSE_PAREN,
    IDENTIFIER,
    LET,
    EQUALS,
    PLUS,
};

struct Token {
    TokenType type;
    std::optional<std::string> value {};
};

class Tokenizer {
    public:
        inline explicit Tokenizer(std::string src)
            : m_src(std::move(src))
        {
        }

        inline std::vector<Token> tokenize() {
            std::string buf;
            std::vector<Token> tokens;

            while (peek().has_value()) {
                if (std::isalpha(peek().value())) {
                    buf.push_back(consume());

                    while (peek().has_value() && std::isalnum(peek().value())) {
                        buf.push_back(consume());
                    }

                    if (buf == "exit") {
                        tokens.push_back({ .type = TokenType::EXIT });
                        buf.clear();

                        continue;
                    } else if (buf == "let") {
                        tokens.push_back({ .type = TokenType::LET });
                        buf.clear();

                        continue;
                    } else {
                        tokens.push_back({ .type = TokenType::IDENTIFIER, .value = buf });
                        buf.clear();
                        continue;
                    }
                } else if (std::isdigit(peek().value())) {
                    buf.push_back(consume());
                    while (peek().has_value() && std::isdigit(peek().value())) {
                        buf.push_back(consume());
                    }

                    tokens.push_back({ .type = TokenType::INT_LITERAL, .value = buf });
                    buf.clear();

                    continue;
                } else if (peek().value() == '(') {
                    consume();
                    tokens.push_back({ .type = TokenType::OPEN_PAREN });
                } else if (peek().value() == ')') {
                    consume();
                    tokens.push_back({ .type = TokenType::CLOSE_PAREN });
                } else if (peek().value() == ';') {
                    consume();
                    tokens.push_back({ .type = TokenType::SEMICOLON });

                    continue;
                } else if (peek().value() == '=') {
                    consume();
                    tokens.push_back({ .type = TokenType::EQUALS });

                    continue;

                } else if (peek().value() == '+') {
                    consume();
                    tokens.push_back({ .type = TokenType::PLUS });

                    continue;
                } else if (std::isspace(peek().value())) {
                    consume();
                    continue;
                }
            }

            m_current_index = 0;

            return tokens;
        }

    private:
        [[nodiscard]] inline std::optional<char> peek(int peek_ahead = 0) const {
            if (m_current_index + peek_ahead >= m_src.length()) {
                return {};
            }

            return m_src.at(m_current_index + peek_ahead);
        }

        inline char consume() {
            return m_src.at(m_current_index++);
        }

        const std::string m_src;
        size_t m_current_index = 0;
};
