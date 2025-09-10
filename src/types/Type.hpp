#pragma once
#include <memory>
#include <unordered_map>
#include <string>

namespace miniml {
  // --- Type kinds
  enum class TKind { INT, BOOL, VAR, FUN };

  // Forward declaration
  struct Type;
  using TypePtr = std::shared_ptr<Type>;

  // --- Type representation
  struct Type {
    TKind k;

    // For VAR
    struct Var { int id; };
    // For FUN
    struct Fun { TypePtr a; TypePtr b; };

    union {
      Var v;
      Fun f;
    };

    // Constructors
    explicit Type(TKind kind) : k(kind) {
      if (k == TKind::VAR) v = Var{-1};
      if (k == TKind::FUN) f = Fun{nullptr,nullptr};
    }

    // Need manual destructor (union with non-trivial types)
    ~Type() {}

    // Factories
    static TypePtr tInt()  { return std::make_shared<Type>(TKind::INT); }
    static TypePtr tBool() { return std::make_shared<Type>(TKind::BOOL); }
    static TypePtr tVar(int id) {
      auto t = std::make_shared<Type>(TKind::VAR);
      t->v.id = id;
      return t;
    }
    static TypePtr tFun(TypePtr a, TypePtr b) {
      auto t = std::make_shared<Type>(TKind::FUN);
      t->f.a = a;
      t->f.b = b;
      return t;
    }
  };

  // --- Substitution: mapping type variable IDs to Types
  struct Subst {
    std::unordered_map<int, TypePtr> m;

    // Apply substitution to a type
    TypePtr apply(TypePtr t) const {
      if (!t) return t;
      switch (t->k) {
        case TKind::INT:
        case TKind::BOOL:
          return t;

        case TKind::VAR: {
          auto it = m.find(t->v.id);
          if (it != m.end()) {
            return apply(it->second); // recursive apply
          }
          return t;
        }

        case TKind::FUN: {
          auto a = apply(t->f.a);
          auto b = apply(t->f.b);
          if (a == t->f.a && b == t->f.b) {
            return t; // unchanged
          }
          return Type::tFun(a, b);
        }
      }
      return t; // unreachable
    }

    // Compose with another substitution (apply s2 then this)
    void compose(const Subst& s2) {
      for (auto& kv : m) {
        kv.second = s2.apply(kv.second);
      }
      for (auto& kv : s2.m) {
        m[kv.first] = kv.second;
      }
    }
  };

  // --- Type errors (shared between Unify and Infer)
  struct TypeError : std::runtime_error {
    using std::runtime_error::runtime_error;
  };
} // namespace miniml
