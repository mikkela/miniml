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
    struct EVar; struct ELitInt; struct ELitBool; struct ELitTuple;
    struct ELam; struct EApp; struct ELet; struct EIf; struct EUnOp; struct EBinOp;

    using Expr = std::variant<EVar, ELitInt, ELitBool, ELitTuple, ELam, EApp, ELet, EIf, EUnOp, EBinOp>;
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
    // An boolean literal
    struct ELitBool {
        SrcLoc loc;
        bool value;
    };
    // A tuple literal
    struct ELitTuple {
        SrcLoc loc;
        std::vector<ExprPtr> elems;
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

    // Unary operator expression
    enum class UnOp { Not };
    struct EUnOp {
        SrcLoc loc;
        UnOp op;
        ExprPtr expr;
    };

    // Binary operator expression
    enum class BinOp { Add, Sub, Mul, Div, Eq, Neq, Lt, Le, Gt, Ge, And, Or };
    struct EBinOp {
        SrcLoc loc;
        BinOp op;
        ExprPtr lhs;
        ExprPtr rhs;
    };

    // --- Convenience constructors (keep API you already used)
    inline ExprPtr var(std::string n, SrcLoc loc)            { return make_expr<EVar>(loc, std::move(n)); }
    inline ExprPtr lit_int(std::int64_t v, SrcLoc loc)       { return make_expr<ELitInt>(loc, v); }
    inline ExprPtr lit_bool(bool v, SrcLoc loc)       { return make_expr<ELitBool>(loc, v); }
    inline ExprPtr lam(std::string x, ExprPtr b, SrcLoc loc) { return make_expr<ELam>(loc, std::move(x), std::move(b)); }
    inline ExprPtr app(ExprPtr f, ExprPtr a, SrcLoc loc)     { return make_expr<EApp>(loc, std::move(f), std::move(a)); }
    inline ExprPtr let_(std::string x, ExprPtr r, ExprPtr b, SrcLoc loc) { return make_expr<ELet>(loc, std::move(x), std::move(r), std::move(b)); }
    inline ExprPtr if_(ExprPtr c, ExprPtr t, ExprPtr e, SrcLoc loc)      { return make_expr<EIf>(loc, std::move(c), std::move(t), std::move(e)); }
    inline ExprPtr unop(UnOp op, ExprPtr e, SrcLoc loc) { return make_expr<EUnOp>(loc, op, std::move(e)); }
    inline ExprPtr binop(BinOp op, ExprPtr l, ExprPtr r, SrcLoc loc) { return make_expr<EBinOp>(loc, op, std::move(l), std::move(r)); }
    inline ExprPtr lit_tuple(std::vector<ExprPtr> t, SrcLoc loc) { return make_expr<ELitTuple>(loc, std::move(t)); }

} // namespace miniml
