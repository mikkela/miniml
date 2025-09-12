#include "Unify.hpp"

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
            case TKind::TUPLE:                             // ← add this case
                for (auto& e : t->tupleElems) if (occursIn(varId, e, s)) return true;
                return false;
        }
        return false;
    }

    static Subst bindVar(int varId, TypePtr t, const SrcLoc& where) {
        if (t->k == TKind::VAR && t->v.id == varId) return {};
        if (occursIn(varId, t, {})) {
            throw TypeError(where.file + ":" + std::to_string(where.line) + ":" +
                            std::to_string(where.col) + ": occurs check fails");
        }
        Subst s;
        s.m.emplace(varId, t);
        return s;
    }

    Subst unify(TypePtr t1, TypePtr t2, const SrcLoc& where) {
        Subst s;
        // recursive lambda captures by reference
        std::function<Subst(TypePtr, TypePtr)> go = [&](TypePtr a, TypePtr b) -> Subst {
            a = s.apply(a);
            b = s.apply(b);

            if (a->k == TKind::VAR) {
                auto si = bindVar(a->v.id, b, where);
                s.compose(si);
                return si;
            }
            if (b->k == TKind::VAR) {
                auto si = bindVar(b->v.id, a, where);
                s.compose(si);
                return si;
            }

            if (a->k == TKind::INT && b->k == TKind::INT) return {};
            if (a->k == TKind::BOOL && b->k == TKind::BOOL) return {};

            if (a->k == TKind::FUN && b->k == TKind::FUN) {
                auto s1 = go(a->f.a, b->f.a);
                (void)s1;
                auto s2 = go(a->f.b, b->f.b);
                (void)s2;
                return {};
            }

            if (a->k == TKind::TUPLE && b->k == TKind::TUPLE) {
                if (a->tupleElems.size() != b->tupleElems.size()) {
                    throw TypeError(where.file + ":" + std::to_string(where.line) + ":" +
                                    std::to_string(where.col) + ": tuple arity mismatch");
                }
                for (size_t i = 0; i < a->tupleElems.size(); ++i) {
                    // each unify updates the outer 's' via s.compose(...)
                    (void) go(a->tupleElems[i], b->tupleElems[i]);
                }
                return {};
            }
            throw TypeError(where.file + ":" + std::to_string(where.line) + ":" +
                            std::to_string(where.col) + ": type mismatch during unification");
        };

        go(t1, t2);
        return s;
    }

} // namespace miniml
