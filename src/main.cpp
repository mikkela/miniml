#include <iostream>
#include "ast/Nodes.hpp"
#include "types/TypeCheck.hpp"
#include "ir/IR.hpp"

int main(int argc, char** argv) {
    using namespace miniml;

    // Example program: let id = \x -> x in id 42
    ExprPtr idLam = lam("x", var("x"));
    ExprPtr prog = let_("id", idLam, app(var("id"), lit_int(42)));

    TypeEnv tenv;
    InferState st;
    auto ty = infer(tenv, st, *prog);

    std::cout << "Type of program: " << ty->str() << "\n";

    // Lower to simple IR
    IRProgram ir;
    IRBuilder irb(ir);
    auto v = irb.gen(*prog);
    irb.bind(v, IRValue::Imm(0)); // placeholder write to last tmp
    irb.emit(IRInstr{IROp::RET, v, IRValue{}});

    std::cout << "\n=== IR ===\n";
    for (auto& f : ir.funcs) {
        std::cout << "func " << f.name << ":\n";
        for (auto& ins : f.body) {
            std::cout << "  " << ins.str() << "\n";
        }
    }
    return 0;
}
