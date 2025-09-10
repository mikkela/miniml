#pragma once
#include <string>
#include <variant>
#include <stdexcept>
#include "../ast/Nodes.hpp"
#include "EnvStack.hpp"

namespace miniml {

// visit helper
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct ScopeError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct ScopeConfig {
  bool warn_on_shadow = false;
  void (*on_warning)(const std::string& msg) = nullptr;  // optional hook
};

class ScopeChecker {
public:
  explicit ScopeChecker(ScopeConfig cfg = {}) : cfg_(cfg) {
    // Seed prelude here if you have one:
    // env_.bind("print", {/*loc*/});
  }

  void check(const ExprPtr& e) { check_expr(*e); }

  // expose env if you need to reuse it (e.g., for later passes)
  const EnvStack& env() const { return env_; }

private:
  ScopeConfig cfg_;
  EnvStack env_;

  [[noreturn]] static void unbound(const std::string& name, const SrcLoc& useLoc) {
    throw ScopeError(useLoc.file + ":" + std::to_string(useLoc.line) + ":" +
                     std::to_string(useLoc.col) + ": unbound variable '" + name + "'");
  }

  void bind_with_warning(const std::string& name, const SrcLoc& where) {
    if (auto prev = env_.bind(name, where)) {
      if (cfg_.warn_on_shadow && cfg_.on_warning) {
        cfg_.on_warning(
          where.file + ":" + std::to_string(where.line) + ":" + std::to_string(where.col) +
          ": shadowing '" + name + "' (previously defined at " +
          prev->file + ":" + std::to_string(prev->line) + ":" + std::to_string(prev->col) + ")"
        );
      }
    }
  }

  void check_expr(const Expr& e) {
    std::visit(overloaded{
      [&](const EVar& n) {
        if (!env_.isBound(n.name)) unbound(n.name, n.loc);
      },
      [&](const ELitInt&) { /* ok */ },
      [&](const ELitBool&) { /* ok */ },
      [&](const ELam& n) {
        env_.push();
        bind_with_warning(n.param, n.loc);
        check_expr(*n.body);
        env_.pop();
      },
      [&](const EApp& n) {
        check_expr(*n.fn);
        check_expr(*n.arg);
      },
      [&](const EIf& n) {
        check_expr(*n.cond);
        check_expr(*n.thenE);
        check_expr(*n.elseE);
      },
      [&](const ELet& n) {
        // Non-recursive let: x not visible in rhs
        check_expr(*n.rhs);
        env_.push();
        bind_with_warning(n.name, n.loc);
        check_expr(*n.body);
        env_.pop();
      },
      [&](const EUnOp& n) {
        check_expr(*n.expr);
      },
      [&](const EBinOp& n) {
        check_expr(*n.lhs);
        check_expr(*n.rhs);
      }
    }, e);
  }
};

} // namespace miniml
