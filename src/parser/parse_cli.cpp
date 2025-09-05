#include <iostream>
#include <string>
#include "parser/parse_to_ast.hpp"
//#include "ast/Pretty.hpp"   // if you have a pretty-printer; else skip

int main(int argc, char** argv) {
    try {
        std::string code = (argc > 1) ? std::string(argv[1]) : "let id = \\x -> x in id 42";
        // If you want to read a file, add a file load here; this uses arg as code snippet.
        auto ast = miniml::parse_to_ast(code);
        // If you don’t have Pretty, just confirm success:
        std::cout << "Parsed OK\n";
        // std::cout << miniml::pretty(ast) << "\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << "\n";
        return 1;
    }
}