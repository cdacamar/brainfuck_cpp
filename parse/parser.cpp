#include <sstream>
#include <string> // for std::literals::string_literals

#include <lex/lexer.h>
#include <parse/parser.h>

// commands
#include <cmd/arithmetic.h>
#include <cmd/io.h>
#include <cmd/loop.h>
#include <cmd/shift.h>

namespace { // utils

std::string make_error(const bf::lex::token& tok) {
  std::stringstream ss;
  ss << tok.pos.name << '(' << tok.pos.line << ':' << tok.pos.col << ')' << ": unexpected token: " << static_cast<char>(tok.type);
  return ss.str();
}

}

namespace bf {

namespace parse {

bool parser::expect_(lex::token_t tok) {
  if (c_tok_.type != tok) {
    using namespace std::literals;
    auto str = make_error(c_tok_);
    str += " got: "s + static_cast<char>(c_tok_.type);
    return false;
  }
  next_token_();
  return true;
}

std::unique_ptr<cmd::loop> parser::parse_loop_() {
  cmd::loop::cmd_list_t commands;

  if (!expect_(lex::token_t::l_bracket)) return nullptr; // eat '['

  while (c_tok_.type != lex::token_t::r_bracket) {
    switch (c_tok_.type) {
      case lex::token_t::l_shift:
        commands.emplace_back(std::make_unique<cmd::shift>(cmd::shift::shift_type::left));
        next_token_();
        break;
      case lex::token_t::r_shift:
        commands.emplace_back(std::make_unique<cmd::shift>(cmd::shift::shift_type::right));
        next_token_();
        break;
      case lex::token_t::inc:
        commands.emplace_back(std::make_unique<cmd::arithmetic>(cmd::arithmetic::arith_type::add));
        next_token_();
        break;
      case lex::token_t::dec:
        commands.emplace_back(std::make_unique<cmd::arithmetic>(cmd::arithmetic::arith_type::sub));
        next_token_();
        break;
      case lex::token_t::out:
        commands.emplace_back(std::make_unique<cmd::io>(cmd::io::io_type::out));
        next_token_();
        break;
      case lex::token_t::in:
        commands.emplace_back(std::make_unique<cmd::io>(cmd::io::io_type::in));
        next_token_();
        break;
      case lex::token_t::l_bracket:
        {
          auto loop = parse_loop_();
          if (loop) commands.emplace_back(std::move(loop));
          else      return nullptr;
        }
        break;
      case lex::token_t::eof:
        errors_.emplace_back("unexpected EOF");
        return nullptr; // we can't hit this while in a loop
    }
  }

  if (!expect_(lex::token_t::r_bracket)) return nullptr; // eat ']'

  return std::make_unique<cmd::loop>(std::move(commands));
}

void parser::next_token_() {
  c_tok_ = lexer_.next_token();
}

prog::program parser::parse() {
  prog::program p;

  next_token_();
  do {
    switch (c_tok_.type) {
      case lex::token_t::l_shift:
        p.emplace_back(std::make_unique<cmd::shift>(cmd::shift::shift_type::left));
        next_token_();
        break;
      case lex::token_t::r_shift:
        p.emplace_back(std::make_unique<cmd::shift>(cmd::shift::shift_type::right));
        next_token_();
        break;
      case lex::token_t::inc:
        p.emplace_back(std::make_unique<cmd::arithmetic>(cmd::arithmetic::arith_type::add));
        next_token_();
        break;
      case lex::token_t::dec:
        p.emplace_back(std::make_unique<cmd::arithmetic>(cmd::arithmetic::arith_type::sub));
        next_token_();
        break;
      case lex::token_t::out:
        p.emplace_back(std::make_unique<cmd::io>(cmd::io::io_type::out));
        next_token_();
        break;
      case lex::token_t::in:
        p.emplace_back(std::make_unique<cmd::io>(cmd::io::io_type::in));
        next_token_();
        break;
      case lex::token_t::l_bracket:
        {
          auto loop = parse_loop_();
          if (loop) {
            p.emplace_back(std::move(loop));
          }
          else {
            p.clear(); // ill-formed
            c_tok_.type = lex::token_t::eof; // move to eof
          }
        }
        break;
      case lex::token_t::r_bracket:
        errors_.emplace_back(make_error(c_tok_));
        p.clear(); // ill-formed
        c_tok_.type = lex::token_t::eof;
        break;
    }
  } while (c_tok_.type != lex::token_t::eof);

  return p;
}

} // namespace parse

} // namespace bf