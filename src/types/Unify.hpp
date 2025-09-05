#pragma once
#include "Type.hpp"
#include <stdexcept>
#include <unordered_set>

namespace miniml {

struct UnifyError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

inline bool occurs(int tv, TypePtr t){
    switch (t->k) {
        case TKind::VAR: return t->v.id == tv;
        case TKind::FUN: return occurs(tv, t->f.a) || occurs(tv, t->f.b);
        default: return false;
    }
}

inline Subst unify(TypePtr a, TypePtr b){
    if (a->k == TKind::VAR) {
        if (b->k == TKind::VAR && a->v.id == b->v.id) return Subst{};
        if (occurs(a->v.id, b)) throw UnifyError("occurs check");
        return Subst{{{a->v.id, b}}};
    }
    if (b->k == TKind::VAR) return unify(b, a);

    if (a->k == TKind::INT && b->k == TKind::INT) return Subst{};
    if (a->k == TKind::BOOL && b->k == TKind::BOOL) return Subst{};

    if (a->k == TKind::FUN && b->k == TKind::FUN) {
        auto s1 = unify(a->f.a, b->f.a);
        auto s2 = unify(s1.apply(a->f.b), s1.apply(b->f.b));
        s1.compose(s2);
        return s1;
    }
    throw UnifyError("cannot unify " + a->str() + " with " + b->str());
}

} // namespace miniml
