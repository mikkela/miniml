#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include "Type.hpp"

namespace miniml {

    // Polymorf type-miljø: navn -> scheme
    using TypeEnv = std::unordered_map<std::string, TypeScheme>;

    // Frisk TVar-id (global counter; implementeret i Scheme.cpp)
    int freshTypeVarId();

    // Instantiate ∀-kvantificerede variabler i sigma til friske TVar’er
    TypePtr instantiate(const TypeScheme& sigma);

    // Generalize: kvantificér ftv(t) \ ftv(Gamma)
    TypeScheme generalize(const TypeEnv& gamma, TypePtr t);

    // Free type vars i et helt miljø
    std::unordered_set<int> ftv(const TypeEnv& gamma);

    // Apply substitution til hele miljøet (maskerer kvantificerede vars i hvert scheme)
    TypeEnv apply(const Subst& s, const TypeEnv& gamma);

} // namespace miniml
