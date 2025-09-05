// src/types/TypeCheck.hpp
#pragma once
#include <variant>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "../ast/Nodes.hpp"
#include "Type.hpp"
//#include "TypeEnv.hpp"

namespace miniml {
  // ------------------------
  // utility: std::visit helper
  // ------------------------
  template<class... Ts>
  struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts>
  overloaded(Ts...) -> overloaded<Ts...>;

  // ------------------------
  // Scope checking (unbound vars)
  // ------------------------
  inline void check_scopes_impl(const Expr& e,
                                std::vector<std::unordered_map<std::string, bool>>& scopes) {
    std::visit(overloaded{
      [&](const EVar& n) {
        // lookup n.name in scope stack
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
          if (it->count(n.name)) return;
        }
        throw std::runtime_error(n.loc.file + ":" + std::to_string(n.loc.line) + ":" +
                                 std::to_string(n.loc.col) + ": unbound variable '" + n.name + "'");
      },
      [&](const ELitInt&) {
        // ok
      },
      [&](const ELam& n) {
        scopes.push_back({});
        scopes.back()[n.param] = true;
        check_scopes_impl(*n.body, scopes);
        scopes.pop_back();
      },
      [&](const EApp& n) {
        check_scopes_impl(*n.fn, scopes);
        check_scopes_impl(*n.arg, scopes);
      },
      [&](const ELet& n) {
        check_scopes_impl(*n.rhs, scopes);
        scopes.push_back({});
        scopes.back()[n.name] = true;
        check_scopes_impl(*n.body, scopes);
        scopes.pop_back();
      },
      [&](const EIf& n) {
        check_scopes_impl(*n.cond, scopes);
        check_scopes_impl(*n.thenE, scopes);
        check_scopes_impl(*n.elseE, scopes);
      }
    }, e);
  }

  inline void check_scopes(const ExprPtr& e) {
    std::vector<std::unordered_map<std::string, bool>> scopes;
    scopes.push_back({}); // global
    check_scopes_impl(*e, scopes); // NOTE: visit the variant directly (no .v)
  }

  // ------------------------
  // Type inference API (placeholder for now)
  // ------------------------
  //
  // This is a minimal stub so your project compiles while you finish the real infer/unify.
  // It returns Int for everything, which is fine if the tests don't assert specific types yet.
  // Replace with your real HM-lite once ready.
  //
  inline TypePtr infer(const ExprPtr& /*e*/) {
    return Type::tInt(); // TODO: implement algorithm W (lite) and return real types
  }
} // namespace miniml
