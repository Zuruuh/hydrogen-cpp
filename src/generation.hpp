#pragma once

#include "./parser.hpp"
#include <cstdlib>
#include <sstream>
#include <sys/types.h>
#include <unordered_map>
#include <variant>

class Generator {
public:
    inline explicit Generator(NodeProgram root) : m_program(std::move(root)) {}

    void gen_expr(const NodeExpr& expr) {
        struct ExprVisitor {
            Generator* gen;

            void operator()(const NodeExprIntLit& expr_int_lit) const {
                gen->m_output << "    mov rax, " << expr_int_lit.int_lit.value.value() << "\n";
                gen->push("rax");
            }

            void operator()(const NodeExprIdent& expr_ident) const {
                if (!gen->m_vars.contains(expr_ident.ident.value.value())) {
                    std::cerr << "Undeclared identifier: " << expr_ident.ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                const Var& var = gen->m_vars.at(expr_ident.ident.value.value());
                std::stringstream offset;
                offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_index - 1) * 8 << "]";
                gen->push(offset.str());
            }
        };

        ExprVisitor visitor { .gen = this };
        std::visit(visitor, expr.expression);
    }

    void gen_stmt(const NodeStmt& stmt) {
        struct StmtVisitor {
            Generator* gen;

            void operator()(const NodeStmtExit& stmt_exit) const {
                gen->gen_expr(stmt_exit.expr);
                gen->m_output << "    mov rax, 60\n";
                gen->pop("rdi");
                gen->m_output << "    syscall\n";
            }
            void operator()(const NodeStmtLet& stmt_let) const {
                if (gen->m_vars.contains(stmt_let.ident.value.value())) {
                    std::cerr << "Identifier already used: " << stmt_let.ident.value.value() << std::endl;
                    exit(EXIT_FAILURE);
                }

                gen->m_vars.insert({ stmt_let.ident.value.value(), Var { .stack_index = gen->m_stack_size } });
                gen->gen_expr(stmt_let.expr);
            }
        };

        StmtVisitor visitor { .gen = this };
        std::visit(visitor, stmt.statement);
    }

    [[nodiscard]] inline std::string generate()
    {
        m_output << "global _start\nstart:\n";

        for (const NodeStmt& stmt : m_program.statements) {
            gen_stmt(stmt);
        }

        m_output << "    mov rax, 60\n";
        m_output << "    mov rdi, 0\n";
        m_output << "    syscall\n";

        return m_output.str();
    }

private:

    void push(const std::string& registry) {
        m_output << "    push " << registry << "\n";
        m_stack_size++;
    }

    void pop(const std::string& registry) {
        m_output << "    pop " << registry << "\n";
        m_stack_size--;
    }

    struct Var {
        size_t stack_index;
    };

    const NodeProgram m_program;
    std::stringstream m_output;
    size_t m_stack_size = 0;
    std::unordered_map<std::string, Var> m_vars {};
};
