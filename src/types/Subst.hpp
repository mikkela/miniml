#pragma once
#include <unordered_map>
#include <string>
#include "Type.hpp"
#include "Scheme.hpp"

namespace miniml {

    // Substitution: mapping from type variable id to type
    struct Subst {
        std::unordered_map<int, TypePtr> m;

        // Apply to Type
        TypePtr apply(const TypePtr& t) const;

        // Apply to Scheme (mask bound vars)
        TypeScheme apply(const TypeScheme& sc) const;

        // Compose: apply 'other' first, then this (this ∘ other)
        void compose(const Subst& other);
    };

    // Collision-proof helpers (avoid std::apply):
    using Env = TypeEnv;

    inline TypePtr apply_type(const Subst& s, const TypePtr& t) { return s.apply(t); }
    inline TypeScheme apply_scheme(const Subst& s, const TypeScheme& sch) { return s.apply(sch); }
    Env apply_env(const Subst& s, const Env& gamma);

} // namespace miniml
