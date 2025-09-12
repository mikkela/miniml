#pragma once
#include <utility>
#include "../ast/Nodes.hpp"
#include "Scheme.hpp"
#include "Subst.hpp"
#include "Unify.hpp"

namespace miniml {

  struct InferResult {
    Subst subst;
    TypePtr type;
  };

// Infer type of expression under environment 'gamma'.
// Returns {S, T} such that S ∘ gamma ⊢ expr : T
  InferResult infer(const Expr& expr, const TypeEnv& gamma);

  inline InferResult infer(const std::shared_ptr<Expr>& expr, const TypeEnv& gamma) {
    return infer(*expr, gamma);
  }
} // namespace miniml
