#pragma once
#include <string>
#include <memory>
#include <any>
#include "MiniMLBaseVisitor.h"
#include <antlr4-runtime.h>
#include "StrictErrorListener.hpp"
#include "MiniMLLexer.h"
#include "MiniMLParser.h"
#include "AstBuilder.hpp"
#include "../ast/Nodes.hpp"

namespace miniml {

    inline ExprPtr parse_to_ast(const std::string& code) {
        antlr4::ANTLRInputStream input(code);
        MiniMLLexer lexer(&input);
        StrictErrorListener strict;
        lexer.removeErrorListeners();
        lexer.addErrorListener(&strict);

        antlr4::CommonTokenStream tokens(&lexer);
        tokens.fill();

        MiniMLParser parser(&tokens);
        parser.removeErrorListeners();
        parser.addErrorListener(&strict);

        auto* tree = parser.prog();
        AstBuilder builder;
        return std::any_cast<ExprPtr>(builder.visit(tree));
    }

} // namespace miniml
