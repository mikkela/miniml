#pragma once
#include <string>
#include "Nodes.hpp"

namespace miniml {
    inline std::string showLoc(const SrcLoc& L) {
        if (L.file.empty()) return "<unknown>:" + std::to_string(L.line) + ":" + std::to_string(L.col);
        return L.file + ":" + std::to_string(L.line) + ":" + std::to_string(L.col);
    }
} // namespace miniml