#include "Subst.hpp"
#include <algorithm>

namespace miniml {

    TypePtr Subst::apply(const TypePtr& t) const {
        if (!t) return t;
        switch (t->k) {
            case TKind::INT:
            case TKind::BOOL:
              return t;
            case TKind::VAR: {
                auto it = m.find(t->v.id);
                return it == m.end() ? t : apply(it->second);
            }
            case TKind::FUN: {
                auto a = apply(t->f.a);
                auto b = apply(t->f.b);
                if (a.get() == t->f.a.get() && b.get() == t->f.b.get()) return t;
                return Type::tFun(a, b);
            }
            case TKind::TUPLE: {                     // ← add this block
                bool changed = false;
                std::vector<TypePtr> es; es.reserve(t->tupleElems.size());
                for (auto& e : t->tupleElems) {
                    auto ae = apply(e);
                    changed |= (ae.get() != e.get());
                    es.push_back(ae);
                }
                if (!changed) return t;
                return Type::tTuple(std::move(es));
            }
        }
        return t;
    }

    TypeScheme Subst::apply(const TypeScheme& sc) const {
        // mask quantified ids
        Subst masked;
        for (auto& [k, v] : m) {
            // if k is not quantified, keep it
            if (std::find(sc.quant.begin(), sc.quant.end(), k) == sc.quant.end())
                masked.m.emplace(k, v);
        }
        return TypeScheme{ sc.quant, masked.apply(sc.body) };
    }

    void Subst::compose(const Subst& other) {
        // Update our codomain under 'other'
        for (auto& [k, v] : m) {
            m[k] = other.apply(v);
        }
        // Then add mappings from 'other' that we don't already have
        for (auto& [k, v] : other.m) {
            if (!m.count(k)) m.emplace(k, v);
        }
    }

    Env apply_env(const Subst& s, const Env& gamma) {
        Env out; out.reserve(gamma.size());
        for (auto& [x, sch] : gamma) {
            out.emplace(x, s.apply(sch));
        }
        return out;
    }

} // namespace miniml
