#include "Type.hpp"

namespace miniml {

    TypePtr Type::tInt()  { return std::make_shared<Type>(TKind::INT); }
    TypePtr Type::tBool() { return std::make_shared<Type>(TKind::BOOL); }

    TypePtr Type::tVar(int id) {
        auto t = std::make_shared<Type>(TKind::VAR);
        t->v.id = id;
        return t;
    }

    TypePtr Type::tFun(TypePtr a, TypePtr b) {
        auto t = std::make_shared<Type>(TKind::FUN);
        t->f.a = std::move(a);
        t->f.b = std::move(b);
        return t;
    }

    TypePtr Type::tTuple(std::vector<TypePtr> elems) {
        auto t = std::make_shared<Type>(TKind::TUPLE);
        t->tupleElems = std::move(elems);
        return t;
    }

} // namespace miniml
