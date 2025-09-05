#pragma once
#include "../ast/Nodes.hpp"
#include <string>

namespace miniml {
// Placeholder until the real parser is wired (ANTLR or PEG):
inline ExprPtr parse_fake_demo() {
    return let_("id", lam("x", var("x")), app(var("id"), lit_int(42)));
}
} // namespace miniml
