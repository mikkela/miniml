#pragma once
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

namespace miniml {

struct Type;
using TypePtr = std::shared_ptr<Type>;

enum class TKind { INT, BOOL, VAR, FUN };

struct TVar { int id; };
struct TFun { TypePtr a, b; };

struct Type {
    TKind k;
    TVar  v{};
    TFun  f{};

    static TypePtr tInt() { return std::make_shared<Type>(TKind::INT); }
    static TypePtr tBool(){ return std::make_shared<Type>(TKind::BOOL); }
    static TypePtr tVar(int id){ Type t(TKind::VAR); t.v = TVar{id}; return std::make_shared<Type>(t); }
    static TypePtr tFun(TypePtr a, TypePtr b){ Type t(TKind::FUN); t.f = TFun{std::move(a), std::move(b)}; return std::make_shared<Type>(t); }

    explicit Type(TKind k): k(k) {}

    std::string str() const {
        switch (k) {
            case TKind::INT: return "Int";
            case TKind::BOOL: return "Bool";
            case TKind::VAR: return "'" + std::to_string(v.id);
            case TKind::FUN: {
                std::ostringstream os;
                os << "(" << f.a->str() << " -> " << f.b->str() << ")";
                return os.str();
            }
        }
        return "?";
    }
};

struct Subst {
    std::unordered_map<int, TypePtr> m;
    TypePtr apply(TypePtr t) const;
    void compose(const Subst& s);
};

} // namespace miniml
