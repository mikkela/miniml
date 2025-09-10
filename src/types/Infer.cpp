#include "Infer.hpp"
#include "Scheme.hpp"
#include <sstream>
#include <variant>

namespace miniml {
  template<class... Ts>
  struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts>
  overloaded(Ts...) -> overloaded<Ts...>;

  // Compose s2 after s1 (apply s2, then s1)
  static Subst compose(Subst s1, const Subst& s2) { s1.compose(s2); return s1; }

  static InferResult infer_expr(const Expr& e, TypeEnv& gamma);

  InferResult infer(const ExprPtr& e, TypeEnv& gamma) {
    return infer_expr(*e, gamma);
  }

  static TypePtr lookupVar(const std::string& x, const SrcLoc& loc, TypeEnv& gamma) {
    auto it = gamma.find(x);
    if (it == gamma.end()) {
      throw TypeError(loc.file + ":" + std::to_string(loc.line) + ":" +
                      std::to_string(loc.col) + ": unbound variable '" + x + "' in type phase");
    }
    return instantiate(it->second);
  }

  static InferResult infer_expr(const Expr& e, TypeEnv& gamma) {
    return std::visit(overloaded{

      // Var
      [&](const EVar& n) -> InferResult {
        return { lookupVar(n.name, n.loc, gamma), {} };
      },

      // Int literal
      [&](const ELitInt& /*n*/) -> InferResult {
        return { Type::tInt(), {} };
      },

      // Bool literal
      [&](const ELitBool& /*n*/) -> InferResult {
        return { Type::tBool(), {} };
      },

      // Lambda
      [&](const ELam& n) -> InferResult {
        auto a = Type::tVar(freshTypeVarId());
        TypeEnv gamma2 = gamma;
        gamma2[n.param] = TypeScheme{ /*quant*/{}, /*body*/ a };  // monomorf binding
        auto r = infer_expr(*n.body, gamma2);
        auto fun = Type::tFun(r.subst.apply(a), r.type);
        return { fun, r.subst };
      },

      // Application  (note: EApp has field 'fn', not 'fun')
      [&](const EApp& n) -> InferResult {
        auto f = infer_expr(*n.fn,  gamma);
        auto a = infer_expr(*n.arg, gamma);
        auto b = Type::tVar(freshTypeVarId());

        // unify f.type with (a.type -> b)
        auto u = unify(a.subst.apply(f.type),
                       Type::tFun(a.type, b),
                       n.loc);

        Subst s = {};
        s.compose(f.subst);
        s.compose(a.subst);
        s.compose(u);

        return { s.apply(b), s };
      },

      // Let (non-recursive, monomorphic)
      [&](const ELet& n) -> InferResult {
        auto r1 = infer_expr(*n.rhs, gamma);
        TypeEnv gamma1 = miniml::apply(r1.subst, gamma);
        auto t1 = r1.subst.apply(r1.type);
        auto sigma = generalize(gamma1, t1); // generalize here for future use
        TypeEnv gamma2 = gamma1;
        gamma2[n.name] = sigma;
        auto r2 = infer_expr(*n.body, gamma2);

        Subst s = {};
        s.compose(r1.subst);
        s.compose(r2.subst);
        return { r2.type, s };
      },

      // If
      [&](const EIf& n) -> InferResult {
        auto rc = infer_expr(*n.cond, gamma);
        auto u1 = unify(rc.type, Type::tBool(), n.loc);

        Subst s = {};
        s.compose(rc.subst);
        s.compose(u1);

        auto rt = infer_expr(*n.thenE, gamma);
        auto re = infer_expr(*n.elseE, gamma);

        auto u2 = unify(s.apply(rt.type), s.apply(re.type), n.loc);
        s.compose(rt.subst);
        s.compose(re.subst);
        s.compose(u2);

        return { s.apply(rt.type), s };
      },

      // Unary 'not'
      [&](const EUnOp& n) -> InferResult {
        switch (n.op) {
          case UnOp::Not: {
            auto r = infer_expr(*n.expr, gamma);
            auto u = unify(r.type, Type::tBool(), n.loc);
            Subst s{};
            s.compose(r.subst);
            s.compose(u);
            return { Type::tBool(), s };
          }
        }
        // unreachable
        return { Type::tBool(), {} };
      },

      // Binary operators
      [&](const EBinOp& n) -> InferResult {
        auto L = n.loc;
        auto l = infer_expr(*n.lhs, gamma);
        auto r = infer_expr(*n.rhs, gamma);
        auto expect = [&](TypePtr tl, TypePtr tr, TypePtr tres) -> InferResult {
          Subst s{};
          s.compose(l.subst);
          s.compose(r.subst);
          auto u1 = unify(s.apply(l.type), tl, L);
          s.compose(u1);
          auto u2 = unify(s.apply(r.type), tr, L);
          s.compose(u2);
          return { tres, s };
        };
        switch (n.op) {
          case BinOp::Add:
          case BinOp::Sub:
          case BinOp::Mul:
          case BinOp::Div:
            return expect(Type::tInt(), Type::tInt(), Type::tInt());
          case BinOp::Eq:
          case BinOp::Neq:
            // Step 1: ints only (polymorphic eq later)
            return expect(Type::tInt(), Type::tInt(), Type::tBool());
          case BinOp::Lt:
          case BinOp::Le:
          case BinOp::Gt:
          case BinOp::Ge:
            return expect(Type::tInt(), Type::tInt(), Type::tBool());
          case BinOp::And:
          case BinOp::Or:
            return expect(Type::tBool(), Type::tBool(), Type::tBool());
        }
        return { Type::tInt(), {} }; // unreachable default
      }
    }, e);
  }

  // --- pretty print types for debugging/CLI ---
  static void showTypeRec(const TypePtr& t, std::ostringstream& out) {
    switch (t->k) {
      case TKind::INT:  out << "Int"; return;
      case TKind::BOOL: out << "Bool"; return;
      case TKind::VAR:  out << "a" << t->v.id; return;
      case TKind::FUN:
        out << "(";
        showTypeRec(t->f.a, out);
        out << " -> ";
        showTypeRec(t->f.b, out);
        out << ")";
        return;
    }
  }

  std::string showType(const TypePtr& t) {
    std::ostringstream out;
    showTypeRec(t, out);
    return out.str();
  }
} // namespace miniml
