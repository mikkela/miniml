// tests/test_parse_to_ast.cpp
#include <gtest/gtest.h>
#include "parser/parse_to_ast.hpp"
#include "types/TypeCheck.hpp"  // if you already have it

TEST(ParseToAst, LetId) {
    auto ast = miniml::parse_to_ast("let id = \\x -> x in id 42");
    ASSERT_TRUE(ast); // basic sanity
}

TEST(ParseToAst, AppAssoc) {
    auto ast = miniml::parse_to_ast("f a b");
    ASSERT_TRUE(ast);
}