#pragma once
#include <memory>
#include <vector>
#include <new>   // for placement new

namespace miniml {

    // ---------- Type core ----------
    enum class TKind { INT, BOOL, VAR, FUN, TUPLE };

    struct Type;
    using TypePtr = std::shared_ptr<Type>;

    struct Type {
        TKind k;

        struct Var { int id; };
        struct Fun { TypePtr a; TypePtr b; };

        union {
            Var v; // active when k == VAR
            Fun f; // active when k == FUN
        };

        // Tuple payload lives OUTSIDE the union (only used when k == TUPLE)
        std::vector<TypePtr> tupleElems;

        // Disable copy/move to avoid union copy rules (we always use shared_ptr<Type>)
        Type(const Type&) = delete;
        Type& operator=(const Type&) = delete;
        Type(Type&&) = delete;
        Type& operator=(Type&&) = delete;

        explicit Type(TKind kind) : k(kind) {
            switch (k) {
                case TKind::VAR: ::new (&v) Var{-1}; break;                  // placement-new
                case TKind::FUN: ::new (&f) Fun{nullptr, nullptr}; break;    // placement-new
                case TKind::INT:
                case TKind::BOOL:
                case TKind::TUPLE:
                  // nothing to construct in the union
                  break;
            }
        }

        ~Type() {
            // Destroy the active union member explicitly
            switch (k) {
                case TKind::VAR: v.~Var(); break;
                case TKind::FUN: f.~Fun(); break;
                case TKind::INT:
                case TKind::BOOL:
                case TKind::TUPLE:
                  // nothing to destroy
                  break;
            }
        }

        // Factory helpers
        static TypePtr tInt();
        static TypePtr tBool();
        static TypePtr tVar(int id);
        static TypePtr tFun(TypePtr a, TypePtr b);
        static TypePtr tTuple(std::vector<TypePtr> elems);
    };

} // namespace miniml
