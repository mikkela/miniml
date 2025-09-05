#pragma once
#include <string>
#include <vector>
#include <optional>
#include <sstream>
#include "../ast/Nodes.hpp"

namespace miniml {

enum class IROp { MOVI, ADD, CALL, RET };

struct IRValue {
    bool isImm=false;
    long imm=0;
    int temp=-1;

    static IRValue Imm(long v){ IRValue x; x.isImm=true; x.imm=v; return x; }
    static IRValue Tmp(int t){ IRValue x; x.temp=t; return x; }

    std::string str() const {
        if (isImm) return "#" + std::to_string(imm);
        return "%t" + std::to_string(temp);
    }
};

struct IRInstr {
    IROp op;
    IRValue a, b;

    std::string str() const {
        std::ostringstream os;
        switch (op){
            case IROp::MOVI: os << "movi " << a.str() << " -> " << b.str(); break;
            case IROp::ADD:  os << "add "  << a.str() << ", " << b.str(); break;
            case IROp::CALL: os << "call " << a.str(); break;
            case IROp::RET:  os << "ret "  << a.str(); break;
        }
        return os.str();
    }
};

struct IRFunc {
    std::string name = "main";
    std::vector<IRInstr> body;
};

struct IRProgram {
    std::vector<IRFunc> funcs = {IRFunc{}};
};

struct IRBuilder {
    IRProgram& p;
    int nextTemp = 0;
    explicit IRBuilder(IRProgram& p): p(p) {}
    IRValue newTmp(){ return IRValue::Tmp(nextTemp++); }
    void emit(const IRInstr& i){ p.funcs[0].body.push_back(i); }
    void bind(IRValue dst, IRValue src){ emit(IRInstr{IROp::MOVI, src, dst}); }

    IRValue gen(const Expr& e){
        return std::visit([&](auto&& node)->IRValue {
            using T = std::decay_t<decltype(node)>;
            if constexpr (std::is_same_v<T, ELitInt>) {
                auto t = newTmp();
                bind(t, IRValue::Imm(node.value));
                return t;
            } else if constexpr (std::is_same_v<T, EApp>) {
                auto _ = gen(*node.fn); (void)_; // placeholder for callable
                auto arg = gen(*node.arg);
                auto t = newTmp();
                emit({IROp::CALL, arg, {}});
                return t;
            } else if constexpr (std::is_same_v<T, EVar>) {
                auto t = newTmp();
                bind(t, IRValue::Imm(0)); // placeholder
                return t;
            } else if constexpr (std::is_same_v<T, ELam>) {
                auto t = newTmp();
                bind(t, IRValue::Imm(0)); // closures omitted in this toy example
                return t;
            } else if constexpr (std::is_same_v<T, ELet>) {
                auto _ = gen(*node.rhs); (void)_;
                return gen(*node.body);
            } else if constexpr (std::is_same_v<T, EIf>) {
                auto _ = gen(*node.cnd); (void)_;
                auto t1 = gen(*node.thn);
                auto t2 = gen(*node.els);
                return t1.temp>=0 ? t1 : t2;
            } else {
                auto t = newTmp();
                bind(t, IRValue::Imm(0));
                return t;
            }
        }, e.v);
    }
};

} // namespace miniml
