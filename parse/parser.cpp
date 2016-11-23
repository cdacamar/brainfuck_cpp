#include <parse/parser.h>

#include <sstream>
#include <string> // for std::literals::string_literals

#include <lex/lexer.h>

// commands
#include <cmd/arithmetic.h>
#include <cmd/io.h>
#include <cmd/loop.h>
#include <cmd/shift.h>

namespace { // utils

std::string make_error_unexpected(const bf::lex::token& tok) {
  std::stringstream ss;
  ss << tok.pos.name << '(' << tok.pos.line << ':' << tok.pos.col << ')' << ": unexpected token: " << static_cast<char>(tok.type);
  return ss.str();
}

std::string make_error_custom(const bf::lex::token& tok, const std::string& msg) {
  std::stringstream ss;
  ss << tok.pos.name << '(' << tok.pos.line << ':' << tok.pos.col << ')' << ": " << msg << " token: " << static_cast<char>(tok.type);
  return ss.str();
}

}

namespace bf {

namespace parse {

bool parser::expect_(lex::token_t tok) {
  if (c_tok_.type != tok) {
    using namespace std::literals;
    auto str = make_error_unexpected(c_tok_);
    str += " got: "s + static_cast<char>(c_tok_.type);
    return false;
  }
  next_token_();
  return true;
}

std::shared_ptr<cmd::command> parser::parse_stmt_() {
  std::shared_ptr<cmd::command> stmt;

  switch (c_tok_.type) {
    case lex::token_t::l_shift:
      stmt = std::make_shared<cmd::shift>(cmd::shift::shift_type::left);
      break;
    case lex::token_t::r_shift:
      stmt = std::make_shared<cmd::shift>(cmd::shift::shift_type::right);
      break;
    case lex::token_t::inc:
      stmt = std::make_shared<cmd::arithmetic>(cmd::arithmetic::arith_type::add);
      break;
    case lex::token_t::dec:
      stmt = std::make_shared<cmd::arithmetic>(cmd::arithmetic::arith_type::sub);
      break;
    case lex::token_t::out:
      stmt = std::make_shared<cmd::io>(cmd::io::io_type::out);
      break;
    case lex::token_t::in:
      stmt = std::make_shared<cmd::io>(cmd::io::io_type::in);
      break;
    case lex::token_t::l_bracket:
    case lex::token_t::r_bracket:
    case lex::token_t::eof:
      errors_.emplace_back(make_error_unexpected(c_tok_));
      return nullptr; // we can't hit this while in a loop
  }
  next_token_(); // eat 'command'

  return stmt;
}

std::shared_ptr<cmd::loop> parser::parse_loop_() {
  cmd::loop::cmd_list_t commands;

  auto c = c_tok_;
  if (!expect_(lex::token_t::l_bracket)) return nullptr; // eat '['

  while (c_tok_.type != lex::token_t::r_bracket) {
    switch (c_tok_.type) {
      case lex::token_t::l_shift:
      case lex::token_t::r_shift:
      case lex::token_t::inc:
      case lex::token_t::dec:
      case lex::token_t::out:
      case lex::token_t::in:
        commands.emplace_back(parse_stmt_());
        break;
      case lex::token_t::l_bracket:
        {
          auto loop = parse_loop_();
          if (loop) commands.emplace_back(std::move(loop));
          else      return nullptr;
        }
        break;
      case lex::token_t::eof:
        errors_.emplace_back(make_error_custom(c, "error trying to match"));
        errors_.emplace_back("unexpected EOF");
        return nullptr; // we can't hit this while in a loop
      case lex::token_t::r_bracket:
        break; // noop
    }
  }

  if (!expect_(lex::token_t::r_bracket)) return nullptr; // eat ']'

  return std::make_shared<cmd::loop>(std::move(commands));
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
      case lex::token_t::r_shift:
      case lex::token_t::inc:
      case lex::token_t::dec:
      case lex::token_t::out:
      case lex::token_t::in:
        p.emplace_back(parse_stmt_());
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
        errors_.emplace_back(make_error_unexpected(c_tok_));
        p.clear(); // ill-formed
        c_tok_.type = lex::token_t::eof;
        break;
      case lex::token_t::eof:
        break; // noop
    }
  } while (c_tok_.type != lex::token_t::eof);

  return p;
}

} // namespace parse

} // namespace bf
