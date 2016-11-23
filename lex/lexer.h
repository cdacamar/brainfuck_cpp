#pragma once
#include <fstream>

#include <lex/token.h>

namespace bf {

namespace lex {

class lexer {
  std::ifstream file_;
  char          current_;
  position_t    c_pos_;

  void next_c_();
  void update_pos_();
public:
  lexer(const std::string& filename);

  const position_t& position() const { return c_pos_; }
  token next_token();
};

} // namespace lex

} // namespace bf
