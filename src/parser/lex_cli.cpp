#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <antlr4-runtime.h>
#include "StrictErrorListener.hpp"
#include "MiniMLLexer.h"

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

    StrictErrorListener strict;
    lexer.removeErrorListeners();
    lexer.addErrorListener(&strict);

    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();

    for (auto t : tokens.getTokens()) {
      std::cout << std::setw(15) << lexer.getVocabulary().getSymbolicName(t->getType())
                << " : '" << t->getText() << "'\n";
    }
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "LEX FAIL: " << e.what() << "\n";
    return 1;
  }
}
