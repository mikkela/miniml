#pragma once
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <stdexcept>

namespace miniml {

// ---------- Type core ----------
enum class TKind { INT, BOOL, VAR, FUN };

struct Type;
using TypePtr = std::shared_ptr<Type>;

struct Type {
  TKind k;

  struct Var { int id; };
  struct Fun { TypePtr a; TypePtr b; };

  union {
    Var v;
    Fun f;
  };

  explicit Type(TKind kind) : k(kind) {
    if (k == TKind::VAR) v = Var{-1};
    if (k == TKind::FUN) f = Fun{nullptr, nullptr};
  }

  ~Type() {} // (forenkl. layout; vi allokerer nye FUN-noder når vi ændrer)

  static TypePtr tInt()  { return std::make_shared<Type>(TKind::INT); }
  static TypePtr tBool() { return std::make_shared<Type>(TKind::BOOL); }
  static TypePtr tVar(int id) {
    auto t = std::make_shared<Type>(TKind::VAR);
    t->v.id = id;
    return t;
  }
  static TypePtr tFun(TypePtr a, TypePtr b) {
    auto t = std::make_shared<Type>(TKind::FUN);
    t->f.a = std::move(a);
    t->f.b = std::move(b);
    return t;
  }
};

// ---------- Substitution ----------
struct Subst {
  std::unordered_map<int, TypePtr> m;

  TypePtr apply(TypePtr t) const {
    if (!t) return t;
    switch (t->k) {
      case TKind::INT:
      case TKind::BOOL:
        return t;
      case TKind::VAR: {
        auto it = m.find(t->v.id);
        if (it != m.end()) return apply(it->second);
        return t;
      }
      case TKind::FUN: {
        auto a = apply(t->f.a);
        auto b = apply(t->f.b);
        if (a == t->f.a && b == t->f.b) return t;
        return Type::tFun(a, b);
      }
    }
    return t;
  }

  void compose(const Subst& s2) {
    // apply s2 to all current images
    for (auto& kv : m) kv.second = s2.apply(kv.second);
    // then union (s2 overrides)
    for (auto& kv : s2.m) m[kv.first] = kv.second;
  }
};

// ---------- Shared error ----------
struct TypeError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

// ---------- Polymorphic schemes (∀ quant. body) ----------
struct TypeScheme {
  std::vector<int> quant; // kvantificerede TVar-ids
  TypePtr body;           // monotype-krop
};

// FTV for Type
inline void ftv(const TypePtr& t, std::unordered_set<int>& out) {
  if (!t) return;
  switch (t->k) {
    case TKind::INT:
    case TKind::BOOL: return;
    case TKind::VAR:  out.insert(t->v.id); return;
    case TKind::FUN:  ftv(t->f.a, out); ftv(t->f.b, out); return;
  }
}

// FTV for Scheme = ftv(body) \ quant
inline std::unordered_set<int> ftv(const TypeScheme& s) {
  std::unordered_set<int> r;
  ftv(s.body, r);
  for (int q : s.quant) r.erase(q);
  return r;
}

// Subst på Scheme: maskér kvantificerede variabler
inline TypeScheme apply(const Subst& s, const TypeScheme& sc) {
  Subst masked = s;
  for (int q : sc.quant) masked.m.erase(q);
  return TypeScheme{ sc.quant, masked.apply(sc.body) };
}

} // namespace miniml
