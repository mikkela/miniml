#include "Scheme.hpp"
#include "Subst.hpp"
#include <atomic>
#include <algorithm>

namespace miniml {

    // ------- fresh TVar supply -------
    static std::atomic<int> g_fresh{0};
    int freshTypeVarId() { return g_fresh++; }

    // ------- ftv over Type -------
    static void ftvTypeRec(const TypePtr& t, std::unordered_set<int>& out) {
        if (!t) return;
        switch (t->k) {
            case TKind::INT:
            case TKind::BOOL:
              return;
            case TKind::VAR:
                out.insert(t->v.id);
                return;
            case TKind::FUN:
                ftvTypeRec(t->f.a, out);
                ftvTypeRec(t->f.b, out);
                return;
            case TKind::TUPLE:
                for (auto& e : t->tupleElems) ftvTypeRec(e, out);
                return;
        }
    }

    std::unordered_set<int> ftv(const TypePtr& t) {
        std::unordered_set<int> r;
        ftvTypeRec(t, r);
        return r;
    }

    // ------- ftv over Scheme -------
    std::unordered_set<int> ftv(const TypeScheme& s) {
        auto r = ftv(s.body);
        for (int q : s.quant) r.erase(q);
        return r;
    }

    // ------- ftv over Env -------
    std::unordered_set<int> ftv(const TypeEnv& gamma) {
        std::unordered_set<int> r;
        for (auto& [_, sigma] : gamma) {
            auto s = ftv(sigma);
            r.insert(s.begin(), s.end());
        }
        return r;
    }

    // ------- instantiate -------
    TypePtr instantiate(const TypeScheme& sigma) {
        // Create a substitution mapping each quantified var to a fresh tvar
        Subst s;
        for (int q : sigma.quant) {
            s.m.emplace(q, Type::tVar(freshTypeVarId()));
        }
        return s.apply(sigma.body);
    }

    // ------- generalize -------
    TypeScheme generalize(const TypeEnv& gamma, TypePtr t) {
        auto ftv_t = ftv(t);
        auto ftv_g = ftv(gamma);
        std::vector<int> quant;
        quant.reserve(ftv_t.size());
        for (int v : ftv_t) {
            if (!ftv_g.count(v)) quant.push_back(v);
        }
        return TypeScheme{ std::move(quant), std::move(t) };
    }

} // namespace miniml
