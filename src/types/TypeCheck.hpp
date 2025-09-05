#pragma once
#include "Type.hpp"
#include "Unify.hpp"
#include "../ast/Nodes.hpp"
#include <map>
#include <stdexcept>

namespace miniml {

struct InferState {
    int nextVar = 0;
    int fresh() { return nextVar++; }
};

using TypeEnv = std::map<std::string, TypePtr>;

inline TypePtr infer(TypeEnv& env, InferState& st, const Expr& e);

inline TypePtr inferVar(TypeEnv& env, const std::string& name){
    auto it = env.find(name);
    if (it == env.end()) throw std::runtime_error("unbound variable: " + name);
    return it->second;
}

inline TypePtr infer(TypeEnv& env, InferState& st, const Expr& e){
    return std::visit([&](auto&& node)->TypePtr {
        using T = std::decay_t<decltype(node)>;
        if constexpr (std::is_same_v<T, EVar>) {
            return inferVar(env, node.name);
        } else if constexpr (std::is_same_v<T, ELitInt>) {
            return Type::tInt();
        } else if constexpr (std::is_same_v<T, ELam>) {
            auto a = Type::tVar(st.fresh());
            TypeEnv env2 = env;
            env2[node.param] = a;
            auto b = infer(env2, st, *node.body);
            return Type::tFun(a, b);
        } else if constexpr (std::is_same_v<T, EApp>) {
            auto tf = infer(env, st, *node.fn);
            auto ta = infer(env, st, *node.arg);
            auto r = Type::tVar(st.fresh());
            auto s = unify(tf, Type::tFun(ta, r));
            return s.apply(r);
        } else if constexpr (std::is_same_v<T, ELet>) {
            auto trhs = infer(env, st, *node.rhs);
            TypeEnv env2 = env;
            env2[node.name] = trhs; // (no generalization for brevity)
            return infer(env2, st, *node.body);
        } else if constexpr (std::is_same_v<T, EIf>) {
            auto tc = infer(env, st, *node.cnd);
            auto s1 = unify(tc, Type::tBool());
            auto tt = infer(env, st, *node.thn);
            auto te = infer(env, st, *node.els);
            auto s2 = unify(s1.apply(tt), s1.apply(te));
            (void)s2;
            return s1.apply(tt);
        } else {
            static_assert(sizeof(T) == 0, "Unhandled node");
        }
    }, e.v);
}

} // namespace miniml
