#pragma once
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace miniml {
    // --- NEW: source location
    struct SrcLoc {
        std::string file;   // optional; can be empty for "<stdin>"
        int line = 1;       // 1-based
        int col  = 1;       // 1-based
    };

    // Forward-declare
    struct EVar; struct ELitInt; struct ELam; struct EApp; struct ELet; struct EIf;

    using Expr = std::variant<EVar, ELitInt, ELam, EApp, ELet, EIf>;
    using ExprPtr = std::shared_ptr<Expr>;

    // Helper to construct shared_ptr
    template <class T, class... Args>
    inline ExprPtr make_expr(Args&&... args) {
        return std::make_shared<Expr>(T{std::forward<Args>(args)...});
    }

    // An identifier. Looks up its meaning in the current environment (during typecheck / eval).
    struct EVar {
        SrcLoc loc;
        std::string name;
    };
    // An integer literal.
    struct ELitInt {
        SrcLoc loc;
        std::int64_t value;
    };
    // A lambda/function with one parameter. (Currying means multi-arg functions are nested lambdas.)
    struct ELam {
        SrcLoc loc;
        std::string param;
        ExprPtr body;
    };
    // Function application. Left-associative: f a b parses/lowers to EApp(EApp(f,a), b).
    struct EApp {
        SrcLoc loc;
        ExprPtr fn;
        ExprPtr arg;
    };
    // A local binding: let name = rhs in body. Introduces a new scope for body.
    struct ELet {
        SrcLoc loc;
        std::string name;
        ExprPtr rhs;
        ExprPtr body;
    };
    // Conditional expression (not a statement). Both branches are expressions.
    struct EIf {
        SrcLoc loc;
        ExprPtr cond;
        ExprPtr thenE;
        ExprPtr elseE;
    };

    // --- Convenience constructors (keep API you already used)
    inline ExprPtr var(std::string n, SrcLoc loc)            { return make_expr<EVar>(loc, std::move(n)); }
    inline ExprPtr lit_int(std::int64_t v, SrcLoc loc)       { return make_expr<ELitInt>(loc, v); }
    inline ExprPtr lam(std::string x, ExprPtr b, SrcLoc loc) { return make_expr<ELam>(loc, std::move(x), std::move(b)); }
    inline ExprPtr app(ExprPtr f, ExprPtr a, SrcLoc loc)     { return make_expr<EApp>(loc, std::move(f), std::move(a)); }
    inline ExprPtr let_(std::string x, ExprPtr r, ExprPtr b, SrcLoc loc) { return make_expr<ELet>(loc, std::move(x), std::move(r), std::move(b)); }
    inline ExprPtr if_(ExprPtr c, ExprPtr t, ExprPtr e, SrcLoc loc)      { return make_expr<EIf>(loc, std::move(c), std::move(t), std::move(e)); }
} // namespace miniml
