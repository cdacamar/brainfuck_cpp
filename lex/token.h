#pragma once
#include <cstdint>

#include <string>

namespace bf {

namespace lex {

enum class token_t : char {
  l_shift   = '<',
  r_shift   = '>',
  inc       = '+',
  dec       = '-',
  out       = '.',
  in        = ',',
  l_bracket = '[',
  r_bracket = ']',
  eof
};
constexpr char tok_to_c(token_t t) { return static_cast<char>(t); }

struct position_t {
  std::string   name;
  std::uint32_t line = 1u;
  std::uint32_t col  = 0u;

  position_t()                = default;
  position_t(const position_t&) = default;
  position_t(position_t&& mv): name{ std::move(mv.name) }, line{ mv.line }, col{ mv.col } { }

  position_t& operator=(const position_t&) = default;
  position_t& operator=(position_t&&)      = default;
};

struct token {
  position_t pos;
  token_t    type;

  token(position_t pos, token_t type): pos{ std::move(pos) }, type{ type } { }
  token() : type{ token_t::eof } { }
  token(const token&) = default;
  token(token&&)      = default;

  token& operator=(const token&) = default;
  token& operator=(token&&)      = default;
};

} // namespace lex

} // namespace bf
