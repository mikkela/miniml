#include "Unify.hpp"
#include <unordered_set>

namespace miniml {

  static bool occursIn(int varId, TypePtr t, const Subst& s) {
    t = s.apply(t);
    switch (t->k) {
      case TKind::INT:
      case TKind::BOOL:
        return false;
      case TKind::VAR:
        return t->v.id == varId;
      case TKind::FUN:
        return occursIn(varId, t->f.a, s) || occursIn(varId, t->f.b, s);
    }
    return false;
  }

  static Subst bindVar(int varId, TypePtr t, const SrcLoc& where) {
    // Apply current substitution invariants outside; here just check occurs
    if (t->k == TKind::VAR && t->v.id == varId) {
      return Subst{}; // no-op
    }
    // occurs check
    Subst empty{};
    if (occursIn(varId, t, empty)) {
      throw TypeError(where.file + ":" + std::to_string(where.line) + ":" +
                      std::to_string(where.col) + ": occurs check failed: "
                      "cannot construct infinite type");
    }
    Subst s;
    s.m[varId] = t;
    return s;
  }

  Subst unify(TypePtr t1, TypePtr t2, const SrcLoc& where) {
    // Always unify on already-applied forms
    if (!t1 || !t2) return {};
    Subst s; // accumulated
    // inner recursive lambda
    std::function<Subst(TypePtr,TypePtr)> go = [&](TypePtr a, TypePtr b) -> Subst {
      a = s.apply(a);
      b = s.apply(b);
      if (a->k == TKind::VAR) {
        auto add = bindVar(a->v.id, b, where);
        s.compose(add);
        return add;
      }
      if (b->k == TKind::VAR) {
        auto add = bindVar(b->v.id, a, where);
        s.compose(add);
        return add;
      }
      if (a->k == TKind::INT && b->k == TKind::INT) return {};
      if (a->k == TKind::BOOL && b->k == TKind::BOOL) return {};
      if (a->k == TKind::FUN && b->k == TKind::FUN) {
        auto s1 = go(a->f.a, b->f.a);
        // apply s after s1 composed
        auto s2 = go(s.apply(a->f.b), s.apply(b->f.b));
        (void)s1; (void)s2;
        return {};
      }
      throw TypeError(where.file + ":" + std::to_string(where.line) + ":" +
                      std::to_string(where.col) + ": type mismatch during unification");
    };

    go(t1, t2);
    return s;
  }
} // namespace miniml
