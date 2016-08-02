#pragma once
#include <cstdint>

#include <cmd/program.h>

namespace bf {

namespace cmd {

struct shift : command {
  enum class shift_type {
    left,
    right
  } type;
  std::uint32_t amount;

  shift(shift_type type, std::uint32_t amount = 1u) : type{ type }, amount{ amount } { }

  void accept(const visitor& visitor) const override { visitor(*this); }
};

} // namespace cmd

} // namespace bf