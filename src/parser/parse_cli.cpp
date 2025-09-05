#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "parser/parse_to_ast.hpp"

static std::string readAll(const char* path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error(std::string("Cannot open file: ") + path);
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int main(int argc, char** argv) {
    try {
        std::string code;
        std::string filename = "<stdin>";
        if (argc > 1) {
            filename = argv[1];
            code = readAll(argv[1]);
        } else {
            code = "let id = \\x -> x in id 42";
        }

        auto ast = miniml::parse_to_ast(code, filename);
        if (!ast) {
            std::cerr << "Parse produced null AST\n";
            return 2;
        }

        std::cout << "Parsed OK from " << filename << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        return 1;
    }
}