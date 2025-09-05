#include <gtest/gtest.h>
#include <antlr4-runtime.h>
#include "parser/StrictErrorListener.hpp"
#include "MiniMLLexer.h"
#include "MiniMLParser.h"

static void mustParse(const std::string& s) {
  antlr4::ANTLRInputStream input(s);
  MiniMLLexer lexer(&input);
  StrictErrorListener strict;
  lexer.removeErrorListeners();
  lexer.addErrorListener(&strict);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();
  MiniMLParser parser(&tokens);
  parser.removeErrorListeners();
  parser.addErrorListener(&strict);
  auto* t = parser.prog();
  ASSERT_NE(t, nullptr);
}

TEST(Parser, LetLamApp) {
  mustParse("let id = \\x -> x in id 42");
}

TEST(Parser, AppAssociativity) {
  mustParse("f a b");
}

TEST(Parser, IfExpr) {
  mustParse("if f x then 1 else 0");
}

TEST(Parser, FailsOnMissingIn) {
  try {
    mustParse("let id = \\x -> x id 42"); // missing 'in'
    FAIL() << "Expected ParseError";
  } catch (const ParseError&) {
    SUCCEED();
  }
}
