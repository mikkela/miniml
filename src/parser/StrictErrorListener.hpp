#pragma once
#include <antlr4-runtime.h>
#include <stdexcept>

struct ParseError : std::runtime_error {
  using std::runtime_error::runtime_error;
};

class StrictErrorListener : public antlr4::BaseErrorListener {
public:
  explicit StrictErrorListener(std::string file_hint = "<stdin>")
  : file_hint_(std::move(file_hint)) {}

  void syntaxError(antlr4::Recognizer * /*rec*/,
                 antlr4::Token *offendingSymbol,
                 size_t line, size_t charPositionInLine,
                 const std::string &msg, std::exception_ptr) override
  {
    const std::string tok = offendingSymbol ? offendingSymbol->getText() : "<eof>";
    // charPositionInLine is 0-based → add 1 for human-friendly columns
    const std::string where =
      file_hint_ + ":" + std::to_string(line) + ":" + std::to_string(charPositionInLine + 1);
    throw ParseError(where + " near '" + tok + "': " + msg);
  }

private:
  std::string file_hint_;
};
