#pragma once
#include <cstdint>

#include <cmd/program.h>

namespace bf {

namespace cmd {

struct arithmetic : command {
  enum class arith_type {
    add,
    sub
  } type;
  std::uint32_t amount;

  arithmetic(arith_type type, std::uint32_t amount = 1u) : type{ type }, amount{ amount } { }

  void accept(const visitor& visitor) const override { visitor(*this); }
};

} // namespace cmd

} // namespace bf