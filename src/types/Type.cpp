// src/types/Type.cpp

#include "Type.hpp"

namespace miniml {

    static TypePtr apply_one(const Subst& s, TypePtr t){
        switch (t->k) {
            case TKind::INT:
            case TKind::BOOL:
                return t;

            case TKind::VAR: {
                auto it = s.m.find(t->v.id);
                return it==s.m.end()? t : apply_one(s, it->second);
            }

            case TKind::FUN:
                // ✅ Use the factory instead of aggregate-init that doesn’t match constructors
                    return Type::tFun(apply_one(s, t->f.a), apply_one(s, t->f.b));
        }
        return t;
    }

    TypePtr Subst::apply(TypePtr t) const { return apply_one(*this, t); }

    void Subst::compose(const Subst& s2){
        for (auto& [k, v] : m) v = s2.apply(v);
        for (auto& [k, v] : s2.m) m[k] = v;
    }

} // namespace miniml
