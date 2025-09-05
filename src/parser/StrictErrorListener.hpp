#pragma once
#include <antlr4-runtime.h>
#include <stdexcept>

struct ParseError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

class StrictErrorListener : public antlr4::BaseErrorListener {
public:
  void syntaxError(antlr4::Recognizer *rec, antlr4::Token *offendingSymbol,
                   size_t line, size_t charPositionInLine,
                   const std::string &msg, std::exception_ptr) override {
    throw ParseError("syntax error at " + std::to_string(line) + ":" +
                     std::to_string(charPositionInLine) + " - " + msg);
  }
};
