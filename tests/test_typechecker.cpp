#include <gtest/gtest.h>
#include "types/TypeCheck.hpp"
#include "ast/Nodes.hpp"

using namespace miniml;

TEST(Typechecker, Id) {
    TypeEnv env;
    InferState st;
    auto id = lam("x", var("x"));
    auto ty = infer(env, st, *id);
    EXPECT_EQ(ty->k, TKind::FUN);
    EXPECT_EQ(ty->f.a->k, TKind::VAR);
    EXPECT_EQ(ty->f.b->k, TKind::VAR);
}
