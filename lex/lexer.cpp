#include <lex/lexer.h>

namespace bf {

namespace lex {

lexer::lexer(const std::string& filename): 
  file_{ filename },
  current_{ '\0' },
  c_pos_{ } {
  if (!file_) throw std::ifstream::failure{ "could not open file" };

  c_pos_.name = filename;

  next_c_();
}

void lexer::next_c_() {
  update_pos_();
  current_ = static_cast<char>(file_.get());
  while (current_ == '\n') {
    update_pos_();
    current_ = static_cast<char>(file_.get());
  }
}

void lexer::update_pos_() {
  if (file_.eof()) {
    return;
  }

  if (current_ == '\n') {
    ++c_pos_.line;
    c_pos_.col = 1;
  }
  else {
    ++c_pos_.col;
  }
}

token lexer::next_token() {
  while (!file_.eof()) {
    char c = current_;
    auto p = c_pos_;
    next_c_();

    switch (c) {
      case tok_to_c(token_t::l_shift):
      case tok_to_c(token_t::r_shift):
      case tok_to_c(token_t::inc):
      case tok_to_c(token_t::dec):
      case tok_to_c(token_t::out):
      case tok_to_c(token_t::in):
      case tok_to_c(token_t::l_bracket):
      case tok_to_c(token_t::r_bracket):
        return { p, static_cast<token_t>(c) };
      default:
        ;
    }
  }
  return { c_pos_, token_t::eof };
}

} // namespace lex

} // namespace bf