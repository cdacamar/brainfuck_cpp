#pragma once
#include <cmd/program.h>

namespace bf {

namespace cmd {

struct loop : command {
  using cmd_list_t = std::vector<std::unique_ptr<command>>;
  cmd_list_t statements;

  loop(cmd_list_t statements) : statements{ std::move(statements) } { }

  void accept(const visitor& visitor) const override { visitor(*this); }
};

} // namespace cmd

} // namespace bf