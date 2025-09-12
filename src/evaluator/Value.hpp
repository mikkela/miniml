#pragma once
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include "../ast/Nodes.hpp"

namespace miniml {
    using Val = std::variant<
        long,
        bool,
        std::shared_ptr<struct Closure>,
        std::shared_ptr<struct Tuple>>;

    struct EnvV {
        std::unordered_map<std::string, Val> m;
        std::shared_ptr<EnvV> parent;
        bool get(const std::string& k, Val& out) const {
            if (auto it = m.find(k); it != m.end()) { out = it->second; return true; }
            return parent ? parent->get(k, out) : false;
        }
    };

    struct Closure {
        std::string param;
        ExprPtr body;
        std::shared_ptr<EnvV> env;  // captured
    };

    struct Tuple {
        std::vector<Val> elements;
    };

} // namespace miniml
