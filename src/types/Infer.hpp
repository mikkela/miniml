#pragma once
#include <unordered_map>
#include <string>
#include "../ast/Nodes.hpp"
#include "Type.hpp"
#include "Unify.hpp"

namespace miniml {

    /// Type environment: name -> type (monomorphic for Step 1)
    using TypeEnv = std::unordered_map<std::string, TypePtr>;

    struct InferResult {
        TypePtr type;
        Subst   subst;
    };

    /// Infer the type of an expression under environment Γ.
    /// Step 1: monomorphic let (no generalization).
    InferResult infer(const ExprPtr& e, TypeEnv& gamma);

    /// Pretty print a type (Int, Bool, a0, a1, (t1 -> t2)) — helpful for CLI/tests.
    std::string showType(const TypePtr& t);

} // namespace miniml
