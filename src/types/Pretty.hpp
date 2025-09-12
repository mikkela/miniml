#pragma once
#include <sstream>
#include "Type.hpp"

namespace miniml {

    // Precedence levels: 0 = top, 1 = to the left of "->"
    inline void showTypeRec(const TypePtr& t, std::ostringstream& out, int prec = 0) {
        if (!t) { out << "?"; return; }

        switch (t->k) {
            case TKind::INT:  out << "Int";  return;
            case TKind::BOOL: out << "Bool"; return;

            case TKind::VAR: {
                // print as aN (or use Greek if you like)
                out << "a" << t->v.id;
                return;
            }

            case TKind::FUN: {
                // Left-associative printing with minimal parens
                // If we're nested under an arrow on the left, add parens
                if (prec > 0) out << "(";
                showTypeRec(t->f.a, out, /*prec=*/1);
                out << " -> ";
                showTypeRec(t->f.b, out, /*prec=*/0);
                if (prec > 0) out << ")";
                return;
            }

            // If you already added tuples:
            case TKind::TUPLE: {
                out << "(";
                for (size_t i = 0; i < t->tupleElems.size(); ++i) {
                    if (i) out << ", ";
                    showTypeRec(t->tupleElems[i], out, 0);
                }
                out << ")";
                return;
            }
        }
    }

    inline std::string showType(const TypePtr& t) {
        std::ostringstream out;
        showTypeRec(t, out, 0);
        return out.str();
    }

}