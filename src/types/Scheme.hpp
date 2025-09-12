#pragma once
#include <unordered_set>
#include <unordered_map>
#include <string>
#include "Type.hpp"

namespace miniml {

    // ---------- Polymorphic type schemes ----------
    struct TypeScheme {
        // Quantified variables (by id), e.g. forall a b. body
        std::vector<int> quant;
        TypePtr body;
    };

    // Type environment: name -> scheme
    using TypeEnv = std::unordered_map<std::string, TypeScheme>;

    // Fresh type variable ids (supply defined in Scheme.cpp)
    int freshTypeVarId();

    // Instantiate ∀-quantified vars in a scheme to fresh type vars
    TypePtr instantiate(const TypeScheme& sigma);

    // Generalize: quantify ftv(t) \ ftv(Γ)
    TypeScheme generalize(const TypeEnv& gamma, TypePtr t);

    // Free type vars of a Type
    std::unordered_set<int> ftv(const TypePtr& t);

    // Free type vars of a Scheme
    std::unordered_set<int> ftv(const TypeScheme& s);

    // Free type vars of an Env
    std::unordered_set<int> ftv(const TypeEnv& gamma);

} // namespace miniml