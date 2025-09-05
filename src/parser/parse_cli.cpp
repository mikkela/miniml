#include <iostream>
#include <fstream>
#include <sstream>
#include <antlr4-runtime.h>
#include "StrictErrorListener.hpp"
#include "MiniMLLexer.h"
#include "MiniMLParser.h"

int main(int argc, char** argv) {
  try {
    std::string code;
    if (argc > 1) {
      std::ifstream in(argv[1]);
      if (!in) { std::cerr << "Cannot open file: " << argv[1] << "\n"; return 1; }
      std::ostringstream ss; ss << in.rdbuf();
      code = ss.str();
    } else {
      code = "let id = \\x -> x in id 42";
    }

    antlr4::ANTLRInputStream input(code);
    MiniMLLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    StrictErrorListener strict;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&strict);
    tokens.fill();

    MiniMLParser parser(&tokens);
    parser.removeErrorListeners();
    parser.addErrorListener(&strict);

    auto* tree = parser.prog();
    std::cout << tree->toStringTree(&parser) << "\n";
    return 0;
  } catch (const ParseError& e) {
    std::cerr << "PARSE FAIL: " << e.what() << "\n";
    return 2;
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << "\n";
    return 1;
  }
}
