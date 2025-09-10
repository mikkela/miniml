#include "Eval.hpp"
#include <stdexcept>

namespace miniml {

static Val eval1(const Expr& e, std::shared_ptr<EnvV> env);

Val eval(const ExprPtr& e, std::shared_ptr<EnvV> env) {
  return eval1(*e, std::move(env));
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

static Val eval1(const Expr& e, std::shared_ptr<EnvV> env) {
  return std::visit(overloaded{
    [&](const EVar& n) -> Val {
      Val v;
      if (!env->get(n.name, v))
        throw std::runtime_error(n.loc.file+":"+std::to_string(n.loc.line)+":"+std::to_string(n.loc.col)+
                                 ": runtime: unbound variable '"+n.name+"'");
      return v;
    },
    [&](const ELitInt& n) -> Val { return static_cast<long>(n.value); },
    [&](const ELam& n) -> Val {
      return std::make_shared<Closure>(Closure{n.param, n.body, env});
    },
    [&](const EApp& n) -> Val {
      Val fv = eval(n.fn, env);
      Val av = eval(n.arg, env);
      // builtin “closure”? allow function values only:
      if (auto clo = std::get_if<std::shared_ptr<Closure>>(&fv)) {
        auto child = std::make_shared<EnvV>();
        child->parent = (*clo)->env;
        child->m[(*clo)->param] = av;
        return eval((*clo)->body, child);
      }
      throw std::runtime_error(n.loc.file+":"+std::to_string(n.loc.line)+":"+std::to_string(n.loc.col)+
                               ": runtime: trying to call a non-function");
    },
    [&](const EIf& n) -> Val {
      Val cv = eval(n.cond, env);
      bool b = false;
      if (auto pb = std::get_if<bool>(&cv)) b = *pb;
      else if (auto pi = std::get_if<long>(&cv)) b = (*pi != 0); // until you add real booleans
      else throw std::runtime_error("runtime: non-boolean condition");
      return b ? eval(n.thenE, env) : eval(n.elseE, env);
    },
    [&](const ELet& n) -> Val {
      auto v1 = eval(n.rhs, env);
      auto child = std::make_shared<EnvV>();
      child->parent = env;
      child->m[n.name] = v1;
      return eval(n.body, child);
    }
  }, e);
}

std::string showVal(const Val& v) {
  if (auto i = std::get_if<long>(&v))  return std::to_string(*i);
  if (auto b = std::get_if<bool>(&v))  return *b ? "true" : "false";
  if (std::get_if<std::shared_ptr<Closure>>(&v)) return "<fun>";
  return "<unknown>";
}

std::shared_ptr<EnvV> prelude() {
  auto env = std::make_shared<EnvV>();

  // Minimal boolean literals as bindings
  env->m["true"]  = true;
  env->m["false"] = false;

  // Primitive + as curried function: \x -> \y -> x+y
  // Implement primitives as closures that capture a native lambda via a name, or
  // shortcut: interpret names specially; for now keep it simple with “builtins by name”.
  // Quick hack: add ints-only add via a name the parser can resolve:
  // You can later implement a proper builtin closure mechanism.
  return env;
}

} // namespace miniml
