#pragma once

#include "tokenization.hpp"
#include <cstdlib>
#include <optional>

struct NodeExpr {
    Token int_lit;
};

struct NodeExit {
    NodeExpr expr;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
    {
    }

    [[nodiscard]] inline std::optional<NodeExpr> parse_expr() {
        if (peek().has_value() && peek().value().type == TokenType::INT_LITERAL) {
            return NodeExpr { .int_lit = consume() };
        }

        return {};
    }

    inline std::optional<NodeExit> parse() {
        std::optional<NodeExit> exit_node;

        while(peek().has_value()) {
            if (peek().value().type == TokenType::EXIT) {
                consume();
                if (auto node_expr = parse_expr()) {
                    exit_node = NodeExit { .expr = node_expr.value() };
                } else {
                    std::cerr << "Invalid expression" << std::endl;
                    exit(EXIT_FAILURE);
                }

                if (peek().has_value() && peek().value().type == TokenType::SEMICOLON) {
                    consume();
                } else {
                    std::cerr << "You forgot a semicolon" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }

        m_current_index = 0;

        return exit_node;
    }

private:
    [[nodiscard]] inline std::optional<Token> peek(int peek_ahead = 0) const {
        if (m_current_index + peek_ahead >= m_tokens.size()) {
            return {};
        }

        return m_tokens.at(m_current_index + peek_ahead);
    }

    inline Token consume() {
        return m_tokens.at(m_current_index++);
    }


    const std::vector<Token> m_tokens;
    size_t m_current_index = 0;
};
