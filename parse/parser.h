#pragma once
#include <cmd/program.h>
#include <lex/token.h>

namespace bf {

namespace lex { class lexer; }

namespace cmd { struct loop; }

namespace parse {

class parser {
  lex::lexer&              lexer_;
  lex::token               c_tok_;
  std::vector<std::string> errors_;

  bool expect_(lex::token_t tok);
  std::unique_ptr<cmd::command> parse_stmt_();
  std::unique_ptr<cmd::loop>    parse_loop_();
  void next_token_();
public:
  parser(lex::lexer& lexer) : lexer_{ lexer } { }

  prog::program parse();
  const std::vector<std::string>& errors() { return errors_; }
};

} // namespace parse

} // namespace bf