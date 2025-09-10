#include "Scheme.hpp"
#include <atomic>

namespace miniml {

    // ------- fresh TVar supply -------
    static std::atomic<int> g_fresh{0};
    int freshTypeVarId() {
        return g_fresh++;
    }

    // ------- env FTV -------
    std::unordered_set<int> ftv(const TypeEnv& gamma) {
        std::unordered_set<int> r;
        for (auto& [_, sigma] : gamma) {
            auto s = ftv(sigma);
            r.insert(s.begin(), s.end());
        }
        return r;
    }

    // ------- instantiate(∀.body) -------
    TypePtr instantiate(const TypeScheme& sigma) {
        // byg en substitution q -> freshVar()
        Subst s;
        for (int q : sigma.quant) {
            s.m[q] = Type::tVar(freshTypeVarId());
        }
        return s.apply(sigma.body);
    }

    // ------- generalize(Gamma, t) : ∀(ftv(t)\ftv(Gamma)). t -------
    TypeScheme generalize(const TypeEnv& gamma, TypePtr t) {
        std::unordered_set<int> ft_t;  ftv(t, ft_t);
        std::unordered_set<int> ft_g = ftv(gamma);

        std::vector<int> quant;
        quant.reserve(ft_t.size());
        for (int v : ft_t) {
            if (ft_g.find(v) == ft_g.end()) quant.push_back(v);
        }
        return TypeScheme{ std::move(quant), std::move(t) };
    }

    // ------- apply(Subst, Env) -------
    TypeEnv apply(const Subst& s, const TypeEnv& gamma) {
        TypeEnv out;
        out.reserve(gamma.size());
        for (auto& [name, sigma] : gamma) {
            out.emplace(name, apply(s, sigma)); // bruger apply(Subst, TypeScheme) fra Type.hpp
        }
        return out;
    }

} // namespace miniml
