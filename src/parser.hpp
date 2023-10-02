#pragma once

#include "./tokenization.hpp"
#include "arena.hpp"
#include <cstdlib>
#include <optional>
#include <variant>
#include <vector>

struct NodeTermIntLit {
    Token int_lit;
};

struct NodeTermIdent {
    Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd {
    NodeExpr* lhs;
    NodeExpr* rhs;
};

/* struct NodeBinExprMulti { */
/*     NodeExpr* lhs; */
/*     NodeExpr* rhs; */
/* }; */

struct NodeBinExpr {
    /* std::variant<NodeBinExprAdd*, NodeBinExprMulti*> expr; */
    NodeBinExprAdd* expr;
};

struct NodeTerm {
    std::variant<NodeTermIntLit*, NodeTermIdent*> term;
};

struct NodeExpr {
    std::variant<NodeTerm*, NodeBinExpr*> expression;
};

struct NodeStmtLet {
    Token ident;
    NodeExpr* expr;
};

struct NodeStmtExit {
    NodeExpr* expr;
};

struct NodeStmt {
    std::variant<NodeStmtExit*, NodeStmtLet*> statement;
};

struct NodeProgram {
    std::vector<NodeStmt*> statements;
};

class Parser {
    public:
        inline explicit Parser(std::vector<Token> tokens)
            : m_tokens(std::move(tokens)),
            m_allocator(1024 * 1024 * 4)
    {
    }

        /* inline std::optional<NodeBinExpr*> parse_bin_expr()  */

        [[nodiscard]] inline std::optional<NodeTerm*> parse_term() {
            if (auto int_lit = try_consume(TokenType::INT_LITERAL)) {
                auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
                term_int_lit->int_lit = int_lit.value();

                auto term = m_allocator.alloc<NodeTerm>();
                term->term = term_int_lit;

                return term;
            }
            else if (auto ident = try_consume(TokenType::IDENTIFIER)) {
                auto term_ident = m_allocator.alloc<NodeTermIdent>();
                term_ident->ident = ident.value();

                auto term = m_allocator.alloc<NodeTerm>();
                term->term = term_ident;

                return term;
            }

            return {};
        }

        [[nodiscard]] inline std::optional<NodeExpr*> parse_expr() {
            auto term = parse_term();
            if (!term) { return {}; }

            if (try_consume(TokenType::PLUS)) {
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();

                auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();

                auto lhs_expr = m_allocator.alloc<NodeExpr>();
                lhs_expr->expression = term.value();
                bin_expr_add->lhs = lhs_expr;

                if (auto rhs = parse_expr()) {
                    bin_expr_add->rhs = rhs.value();
                    bin_expr->expr = bin_expr_add;

                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->expression = bin_expr;

                    return expr;
                } else {
                    std::cerr << "Expected right hand side expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }

            auto expr = m_allocator.alloc<NodeExpr>();
            expr->expression = term.value();

            return expr;
        }

        [[nodiscard]] inline std::optional<NodeStmt*> parse_stmt() {
            if (
                    peek().value().type == TokenType::EXIT
                    && peek(1).has_value()
                    && peek(1).value().type == TokenType::OPEN_PAREN
               ) {
                consume();
                consume();

                auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
                if (auto node_expr = parse_expr()) {
                    stmt_exit->expr = node_expr.value();
                } else {
                    std::cerr << "Invalid expression" << std::endl;
                    exit(EXIT_FAILURE);
                }

                try_consume(TokenType::CLOSE_PAREN, "Expected `)`");
                try_consume(TokenType::SEMICOLON, "Expected `;`");

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->statement = stmt_exit;

                return stmt;
            } else if (
                    peek().value().type == TokenType::LET
                    && peek(1).has_value()
                    && peek(1).value().type == TokenType::IDENTIFIER
                    && peek(2).has_value()
                    && peek(2).value().type == TokenType::EQUALS
                ) {
                consume();

                auto stmt_let = m_allocator.alloc<NodeStmtLet>();
                stmt_let->ident = consume();
                consume();

                if (auto expr = parse_expr()) {
                    stmt_let->expr = expr.value();
                } else {
                    std::cerr << "Invalid expression" << std::endl;
                    exit(EXIT_FAILURE);
                }

                try_consume(TokenType::SEMICOLON, "Expected `;`");

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->statement = stmt_let;

                return stmt;
            }

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

        inline Token try_consume(TokenType type, const std::string& err_message) {
            if (peek().has_value() && peek().value().type == type) {
                return consume();
            } else {
                std::cerr << err_message << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        inline std::optional<Token> try_consume(TokenType type) {
            if (peek().has_value() && peek().value().type == type) {
                return consume();
            }

            return {};
        }

        const std::vector<Token> m_tokens;
        size_t m_current_index = 0;
        ArenaAllocator m_allocator;
};
