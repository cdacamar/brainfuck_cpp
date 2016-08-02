#pragma once
#include <cmd/program.h>

namespace bf {

namespace cmd {

struct io : command {
  enum class io_type {
    in,
    out
  } type;

  io(io_type type) : type{ type } { }

  void accept(const visitor& visitor) const override { visitor(*this); }
};

} // namespace cmd

} // namespace bf