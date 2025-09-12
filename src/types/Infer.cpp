#include "Infer.hpp"
#include <functional>

namespace miniml {

static InferResult infer_rec(const Expr& e, TypeEnv gamma);

// Helper to compose substitutions (s2 after s1): result applies s2, then s1
static inline Subst compose(Subst s1, const Subst& s2) { s1.compose(s2); return s1; }

static InferResult infer_var(const EVar& n, const TypeEnv& gamma) {
  auto it = gamma.find(n.name);
  if (it == gamma.end()) {
    throw TypeError(n.loc.file + ":" + std::to_string(n.loc.line) + ":" +
                    std::to_string(n.loc.col) + ": unbound variable '" + n.name + "'");
  }
  // instantiate scheme
  auto t = instantiate(it->second);
  return { {}, t };
}

static InferResult infer_int(const ELitInt& n, const TypeEnv&) {
  (void)n;
  return { {}, Type::tInt() };
}

static InferResult infer_bool(const ELitBool& n, const TypeEnv&) {
  (void)n;
  return { {}, Type::tBool() };
}

  static InferResult infer_if(const EIf& n, const TypeEnv& gamma0) {
  // infer condition
  auto rc = infer_rec(*n.cond, gamma0);
  auto s1 = rc.subst;

  // cond : Bool
  auto su = unify(apply_type(s1, rc.type), Type::tBool(), n.loc);
  auto s2 = compose(su, s1);

  // infer then under updated env
  auto rt = infer_rec(*n.thenE, apply_env(s2, gamma0));
  auto s3 = compose(rt.subst, s2);

  // infer else under updated env
  auto re = infer_rec(*n.elseE, apply_env(s3, gamma0));
  auto s4 = compose(re.subst, s3);

  // branches must match
  auto sb = unify(apply_type(s4, rt.type), apply_type(s4, re.type), n.loc);
  auto sall = compose(sb, s4);

  return { sall, apply_type(sall, rt.type) };
}

static InferResult infer_lam(const ELam& n, TypeEnv gamma) {
  // fresh type var for parameter
  int a_id = freshTypeVarId();
  auto a = Type::tVar(a_id);
  // extend env, parameter is monomorphic here
  gamma[n.param] = TypeScheme{ /*quant*/{}, a };
  auto r_body = infer_rec(*n.body, gamma);
  // function type a -> body
  auto funTy = Type::tFun(apply_type(r_body.subst, a), r_body.type);
  return { r_body.subst, funTy };
}

static InferResult infer_app(const EApp& n, const TypeEnv& gamma0) {
  // infer function
  auto r_fun = infer_rec(*n.fn, gamma0);
  auto gamma1 = apply_env(r_fun.subst, gamma0);

  // infer arg under updated env
  auto r_arg = infer_rec(*n.arg, gamma1);
  auto s = compose(r_arg.subst, r_fun.subst);

  // result type is fresh
  auto b = Type::tVar(freshTypeVarId());

  // unify function type with arg -> b
  auto s_u = unify(apply_type(s, r_fun.type), Type::tFun(apply_type(s, r_arg.type), b), n.loc);
  auto s_all = compose(s_u, s);
  return { s_all, apply_type(s_all, b) };
}

static InferResult infer_let(const ELet& n, const TypeEnv& gamma0) {
  // infer RHS
  auto r_rhs = infer_rec(*n.rhs, gamma0);
  auto gamma1 = apply_env(r_rhs.subst, gamma0);

  // generalize the RHS type w.r.t. gamma1
  auto sigma = generalize(gamma1, apply_type(r_rhs.subst, r_rhs.type));

  // extend env and infer body
  TypeEnv gamma2 = gamma1;
  gamma2[n.name] = std::move(sigma);

  auto r_body = infer_rec(*n.body, gamma2);

  // compose substitutions: body after rhs
  auto s_all = compose(r_body.subst, r_rhs.subst);
  return { s_all, r_body.type };
}

static InferResult infer_tuple(const ELitTuple& n, const TypeEnv& gamma0) {
  Subst s;                         // accumulate left-to-right
  std::vector<TypePtr> elemTypes;
  elemTypes.reserve(n.elems.size());

  for (auto& ep : n.elems) {
    auto r = infer_rec(*ep, apply_env(s, gamma0));     // infer under updated env
    s = compose(r.subst, s);                           // compose: new after old
    elemTypes.push_back(apply_type(s, r.type));        // keep types normalized
  }

  return { s, Type::tTuple(std::move(elemTypes)) };
}

static InferResult infer_unop(const EUnOp& n, const TypeEnv& gamma0) {
  auto r = infer_rec(*n.expr, gamma0);
  Subst s = r.subst;

  switch (n.op) {
    case UnOp::Not: {
      // expr : Bool
      auto su = unify(apply_type(s, r.type), Type::tBool(), n.loc);
      s = compose(su, s);
      return { s, Type::tBool() };
    }
  }
  // unreachable for now
  return { s, Type::tBool() };
}

static InferResult infer_binop(const EBinOp& n, const TypeEnv& gamma0) {
  // infer lhs
  auto rl = infer_rec(*n.lhs, gamma0);
  auto s1 = rl.subst;

  // infer rhs under updated env
  auto rr = infer_rec(*n.rhs, apply_env(s1, gamma0));
  auto s2 = compose(rr.subst, s1);

  auto lhsT = apply_type(s2, rl.type);
  auto rhsT = apply_type(s2, rr.type);

  auto needInt = [&](const TypePtr& t) {
    auto su = unify(t, Type::tInt(), n.loc);
    return compose(su, s2);
  };
  auto needBool = [&](const TypePtr& t) {
    auto su = unify(t, Type::tBool(), n.loc);
    return compose(su, s2);
  };

  switch (n.op) {
    case BinOp::Add:
    case BinOp::Sub:
    case BinOp::Mul:
    case BinOp::Div: {
      auto s3 = needInt(lhsT);
      auto s4 = needInt(apply_type(s3, rhsT));
      return { s4, Type::tInt() };
    }

    case BinOp::And:
    case BinOp::Or: {
      auto s3 = needBool(lhsT);
      auto s4 = needBool(apply_type(s3, rhsT));
      return { s4, Type::tBool() };
    }

    case BinOp::Lt:
    case BinOp::Le:
    case BinOp::Gt:
    case BinOp::Ge: {
      auto s3 = needInt(lhsT);
      auto s4 = needInt(apply_type(s3, rhsT));
      return { s4, Type::tBool() };
    }

    case BinOp::Eq:
    case BinOp::Neq: {
      // α × α → Bool (allow any type that can unify)
      auto su = unify(lhsT, rhsT, n.loc);
      auto s3 = compose(su, s2);
      return { s3, Type::tBool() };
    }
  }

  // Fallback (should not happen)
  return { s2, Type::tBool() };
}

static InferResult infer_rec(const Expr& e, TypeEnv gamma) {
  return std::visit([&](auto const& node) -> InferResult {
    using T = std::decay_t<decltype(node)>;
    if constexpr (std::is_same_v<T, ELitInt>) {
      return infer_int(node, gamma);
    } else if constexpr (std::is_same_v<T, ELitBool>) {
      return infer_bool(node, gamma);
    } else if constexpr (std::is_same_v<T, EVar>) {
      return infer_var(node, gamma);
    } else if constexpr (std::is_same_v<T, ELam>) {
      return infer_lam(node, gamma);
    } else if constexpr (std::is_same_v<T, EApp>) {
      return infer_app(node, gamma);
    } else if constexpr (std::is_same_v<T, ELet>) {
      return infer_let(node, gamma);
    } else if constexpr (std::is_same_v<T, ELitTuple>) {
      return infer_tuple(node, gamma);
    } else if constexpr (std::is_same_v<T, EIf>) {
      return infer_if(node, gamma);
    } else if constexpr (std::is_same_v<T, EUnOp>) {
      return infer_unop(node, gamma);
    } else if constexpr (std::is_same_v<T, EBinOp>) {
      return infer_binop(node, gamma);
    } else {
      static_assert(sizeof(T) == 0, "Unhandled Expr alternative in infer_rec");
    }
  }, e);
}

InferResult infer(const Expr& expr, const TypeEnv& gamma) {
  return infer_rec(expr, gamma);
}

} // namespace miniml
