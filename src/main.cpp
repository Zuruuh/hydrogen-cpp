#include <cctype>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

enum class TokenType {
    _RETURN,
    INT_LITERAL,
    SEMICOLON,
};

struct Token {
    TokenType type;
    std::optional<std::string> value {};
};

std::vector<Token> tokenize(const std::string& str) {
    std::vector<Token> tokens;
    std::string buf;

    for (int i = 0; i <str.length(); i++) {
        char c = str.at(i);
        if (std::isalpha(c)) {
            buf.push_back(c);
            i++;

            while(std::isalpha(str.at(i))) {
                buf.push_back(str.at(i));
                i++;
            }
            i--;

            if (buf == "return") {
                tokens.push_back({.type = TokenType::_RETURN});
                buf.clear();
            } else {
                std::cerr << "You messed up lol" << std::endl;

                exit(EXIT_FAILURE);
            }
        } else if (std::isdigit(c)) {
            buf.push_back(c);
            i++;

            while (std::isdigit(str.at(i))) {
                buf.push_back(str.at(i));
                i++;
            }
            i--;

            tokens.push_back({.type = TokenType::INT_LITERAL, .value = buf});
            buf.clear();
        } else if (c == ';') {
            tokens.push_back({.type = TokenType::SEMICOLON });
        } else if (std::isspace(c)) {
            continue;
        } else {
            std::cerr << "You messed up lol" << std::endl;

            exit(EXIT_FAILURE);
        }
    }

    return tokens;
}

std::string tokens_to_asm(const std::vector<Token>& tokens) {
    std::stringstream output;
    output << "global _start\nstart:\n";

    for (int i=0; i< tokens.size(); i++) {
        const Token& token = tokens.at(i);

        if (token.type == TokenType::_RETURN) {
            if (i + 1< tokens.size() && tokens.at(i+1).type == TokenType::INT_LITERAL) {
                if (i + 2 < tokens.size() && tokens.at(i+2).type == TokenType::SEMICOLON) {
                    output << "    mov rax, 60\n";
                    output << "    mov rdi, " << tokens.at(i+1).value.value() << "\n";
                    output << "    syscall";
                }
            }
        }
    }

    return output.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect usage" << std::endl;
        std::cerr << "hydro <input.hy>" << std::endl;

        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    std::vector<Token> tokens = tokenize(contents);

    {
        std::fstream file("./out.asm", std::ios::out);
        file << tokens_to_asm(tokens);
    }

    system("nasm -felf64 out.asm");
    system("ld out.o -o out");

    return EXIT_SUCCESS;
}
