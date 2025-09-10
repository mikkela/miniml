#pragma once
#include "Value.hpp"

namespace miniml {

    // Evaluate expression under environment; call-by-value
    Val eval(const ExprPtr& e, std::shared_ptr<EnvV> env);

    // Helpers to print values (for CLI)
    std::string showVal(const Val& v);

    // Optional: install a few builtins (+, -, *, =, true, false)
    std::shared_ptr<EnvV> prelude();

} // namespace miniml
