#include <cctype>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>

#include "./tokenization.hpp"
#include "./parser.hpp"
#include "./generation.hpp"

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

    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProgram> program = parser.parse_prog();

    if (!program.has_value()) {
        std::cerr << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(program.value());

    {
        std::fstream file("./out.asm", std::ios::out);
        file << generator.generate();
    }

    system("nasm -felf64 out.asm");
    system("ld out.o -o out");

    return EXIT_SUCCESS;
}
