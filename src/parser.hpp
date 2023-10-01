#pragma once

#include "./tokenization.hpp"
#include <cstdlib>
#include <optional>
#include <variant>
#include <vector>

struct NodeExprIntLit {
    Token int_lit;
};

struct NodeExprIdent {
    Token ident;
};

struct NodeExpr {
    std::variant<NodeExprIdent, NodeExprIntLit> expression;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr expr;
};

struct NodeStmtExit {
    NodeExpr expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit, NodeStmtLet> statement;
};

struct NodeProgram {
    std::vector<NodeStmt> statements;
};

class Parser {
public:
    inline explicit Parser(std::vector<Token> tokens)
        : m_tokens(std::move(tokens))
    {
    }

    [[nodiscard]] inline std::optional<NodeExpr> parse_expr() {
        if (peek().has_value() && peek().value().type == TokenType::INT_LITERAL) {
            return NodeExpr { .expression = NodeExprIntLit { .int_lit = consume() } };
        }
        else if (peek().has_value() && peek().value().type == TokenType::IDENTIFIER) {
            return NodeExpr { .expression = NodeExprIdent { .ident = consume() } };
        }

        return {};
    }

    [[nodiscard]] inline std::optional<NodeStmt> parse_stmt() {
        if (
            peek().value().type == TokenType::EXIT
            && peek(1).has_value()
            && peek(1).value().type == TokenType::OPEN_PAREN
           ) {
            consume();
            consume();

            NodeStmtExit stmt_exit;
            if (auto node_expr = parse_expr()) {
                stmt_exit = { .expr = node_expr.value() };
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (peek().has_value() && peek().value().type == TokenType::CLOSE_PAREN) {
                consume();
            } else {
                std::cerr << "Expected `)`" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (peek().has_value() && peek().value().type == TokenType::SEMICOLON) {
                consume();
            } else {
                std::cerr << "You forgot a semicolon" << std::endl;
                exit(EXIT_FAILURE);
            }

            return NodeStmt { .statement = stmt_exit};
        } else if (
            peek().value().type == TokenType::LET
            && peek(1).has_value()
            && peek(1).value().type == TokenType::IDENTIFIER
            && peek(2).has_value()
            && peek(2).value().type == TokenType::EQUALS
        ) {
            consume();
            auto stmt_let = NodeStmtLet { .ident = consume() };
            consume();

            if (auto expr = parse_expr()) {
                stmt_let.expr = expr.value();
            } else {
                std::cerr << "Invalid expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (!peek().has_value() || peek().value().type != TokenType::SEMICOLON) {
                std::cerr << "Expected `;`, got: " << std::endl;
                exit(EXIT_FAILURE);
            }
            consume();

            return NodeStmt { .statement = stmt_let };
        }

        auto pe = peek();

        return {};
    }

    [[nodiscard]] inline std::optional<NodeProgram> parse_prog() {
        NodeProgram prog;

        while (peek().has_value()) {
            if (auto stmt = parse_stmt()) {
                prog.statements.push_back(stmt.value());
            } else {
                std::cerr << "Invalid statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        return prog;
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
