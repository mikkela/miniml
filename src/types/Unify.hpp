#pragma once
#include "../ast/Nodes.hpp"
#include "Type.hpp"

#include <stdexcept>
#include <unordered_set>

namespace miniml {
    struct UnifyError : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    /// Unify two types, returning a substitution S such that S(t1) == S(t2).
    /// Throws TypeError with SrcLoc on mismatch.
    Subst unify(TypePtr t1, TypePtr t2, const SrcLoc& where);
} // namespace miniml
