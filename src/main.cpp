#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "parser/parse_to_ast.hpp"
#include "semantic/ScopeCheck.hpp"   // hvis du valgte mappen "semantic/"
#include "types/Type.hpp"
#include "evaluator/Eval.hpp"       // eval(...) + showVal(...)
#include "types/Scheme.hpp"
#include "types/Unify.hpp"
#include "types/Infer.hpp"
#include "types/Pretty.hpp"
// (ellers "scope/ScopeCheck.hpp")

static std::string readAll(const char* path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error(std::string("Cannot open file: ") + path);
    std::ostringstream ss; ss << in.rdbuf(); return ss.str();
}

int main(int argc, char** argv) {
    try {
        std::string filename = "<stdin>";
        std::string code;
        if (argc > 1) {
            filename = argv[1];
            code = readAll(argv[1]);
        } else {
            // fallback-program hvis ingen fil gives
            code = "let id = \\x -> x in id 42";
        }

        // 1) Parse → AST (med kildelokationer)
        auto ast = miniml::parse_to_ast(code, filename);

        // 2) Navneresolution / scope-check
        miniml::ScopeConfig cfg;
        cfg.warn_on_shadow = true;
        cfg.on_warning = [](const std::string& msg){ std::cerr << "warning: " << msg << "\n"; };

        miniml::ScopeChecker checker(cfg);
        checker.check(ast);

        // 3) Type inference (HM-lite, monomorphic let for now)
        miniml::TypeEnv gamma;        // add prelude bindings here later, if any
        auto ir = miniml::infer(ast, gamma);

        std::cout << "OK: parsed + scope-checked " << filename << "\n";
        std::cout << "Type: " << miniml::showType(ir.type) << "\n";

        // after type inference:
        auto v = miniml::eval(ast, miniml::prelude());
        std::cout << "Value: " << miniml::showVal(v) << "\n";

        return 0;

    } catch (const miniml::TypeError& e) {     // type errors (from unify/infer)
        std::cerr << "Type error: " << e.what() << "\n";
        return 2;
    }catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 1;
    }
}