#include "Infer.hpp"
#include <sstream>
#include <variant>

namespace miniml {

  static int fresh_counter = 0;
  static TypePtr freshTVar() { return Type::tVar(fresh_counter++); }

  template<class... Ts>
  struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts>
  overloaded(Ts...) -> overloaded<Ts...>;

  // rename to avoid clash with std::apply
  static TypePtr applySubst(const Subst& s, TypePtr t) { return s.apply(t); }

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
    return it->second;
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

      // Lambda
      [&](const ELam& n) -> InferResult {
        auto a = freshTVar();
        TypeEnv gamma2 = gamma;
        gamma2[n.param] = a;
        auto r = infer_expr(*n.body, gamma2);
        auto fun = Type::tFun(applySubst(r.subst, a), r.type);
        return { fun, r.subst };
      },

      // Application  (note: EApp has field 'fn', not 'fun')
      [&](const EApp& n) -> InferResult {
        auto f = infer_expr(*n.fn,  gamma);
        auto a = infer_expr(*n.arg, gamma);
        auto b = freshTVar();

        // unify f.type with (a.type -> b)
        auto u = unify(applySubst(a.subst, f.type),
                       Type::tFun(a.type, b),
                       n.loc);

        Subst s = {};
        s.compose(f.subst);
        s.compose(a.subst);
        s.compose(u);

        return { applySubst(s, b), s };
      },

      // Let (non-recursive, monomorphic)
      [&](const ELet& n) -> InferResult {
        auto r1 = infer_expr(*n.rhs, gamma);
        TypeEnv gamma2 = gamma;
        gamma2[n.name] = applySubst(r1.subst, r1.type);
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

        auto u2 = unify(applySubst(s, rt.type), applySubst(s, re.type), n.loc);
        s.compose(rt.subst);
        s.compose(re.subst);
        s.compose(u2);

        return { applySubst(s, rt.type), s };
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
