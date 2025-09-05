#pragma once
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace miniml {

struct Expr;
using ExprPtr = std::shared_ptr<Expr>;

// An identifier. Looks up its meaning in the current environment (during typecheck / eval).
struct EVar { std::string name; };
// An integer literal.
struct ELitInt { long value; };
// A lambda/function with one parameter. (Currying means multi-arg functions are nested lambdas.)
struct ELam { std::string param; ExprPtr body; };
// Function application. Left-associative: f a b parses/lowers to EApp(EApp(f,a), b).
struct EApp { ExprPtr fn; ExprPtr arg; };
// A local binding: let name = rhs in body. Introduces a new scope for body.
struct ELet { std::string name; ExprPtr rhs; ExprPtr body; };
// Conditional expression (not a statement). Both branches are expressions.
struct EIf  { ExprPtr cnd, thn, els; };

struct Expr {
    std::variant<EVar, ELitInt, ELam, EApp, ELet, EIf> v;

    template<class T, class...A>
    static ExprPtr mk(A&&...a) { return std::make_shared<Expr>(T{std::forward<A>(a)...}); }

    explicit Expr(EVar x): v(std::move(x)) {}
    explicit Expr(ELitInt x): v(std::move(x)) {}
    explicit Expr(ELam x): v(std::move(x)) {}
    explicit Expr(EApp x): v(std::move(x)) {}
    explicit Expr(ELet x): v(std::move(x)) {}
    explicit Expr(EIf  x): v(std::move(x)) {}
};

inline ExprPtr var(std::string n){ return Expr::mk<EVar>(std::move(n)); }
inline ExprPtr lit_int(long v){ return Expr::mk<ELitInt>(v); }
inline ExprPtr lam(std::string p, ExprPtr b){ return Expr::mk<ELam>(std::move(p), std::move(b)); }
inline ExprPtr app(ExprPtr f, ExprPtr a){ return Expr::mk<EApp>(std::move(f), std::move(a)); }
inline ExprPtr let_(std::string n, ExprPtr r, ExprPtr b){ return Expr::mk<ELet>(std::move(n), std::move(r), std::move(b)); }
inline ExprPtr if_(ExprPtr c, ExprPtr t, ExprPtr e){ return Expr::mk<EIf>(std::move(c), std::move(t), std::move(e)); }

} // namespace miniml
