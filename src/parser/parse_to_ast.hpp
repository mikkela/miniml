#pragma once
#include <string>
#include <memory>
#include <any>
#include <antlr4-runtime.h>
#include "StrictErrorListener.hpp"
#include "MiniMLLexer.h"
#include "MiniMLParser.h"
#include "AstBuilder.hpp"
#include "../ast/Nodes.hpp"

namespace miniml {
    inline ExprPtr parse_to_ast(const std::string& code, std::string filename = "<stdin>") {
        antlr4::ANTLRInputStream input(code);
        MiniMLLexer lexer(&input);
        StrictErrorListener lexErr(filename);
        lexer.removeErrorListeners();
        lexer.addErrorListener(&lexErr);

        antlr4::CommonTokenStream tokens(&lexer);
        tokens.fill();

        MiniMLParser parser(&tokens);
        StrictErrorListener parseErr(filename);   // <-- and here
        parser.removeErrorListeners();
        parser.addErrorListener(&parseErr);

        auto* tree = parser.prog();
        AstBuilder builder;
        builder.currentFile = filename;
        return std::any_cast<ExprPtr>(builder.visit(tree));
    }
} // namespace miniml
