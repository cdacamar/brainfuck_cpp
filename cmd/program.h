#pragma once
#include <vector>
#include <memory>

namespace bf {

namespace cmd {

struct command {
  struct visitor;

  virtual ~command() { }

  virtual void accept(const visitor&) const = 0;
};

struct arithmetic;
struct io;
struct loop;
struct shift;
struct command::visitor {
  virtual ~visitor() { }

  virtual void operator()(const arithmetic&) const = 0;
  virtual void operator()(const io&)         const = 0;
  virtual void operator()(const loop&)       const = 0;
  virtual void operator()(const shift&)      const = 0;
};

} // namespace cmd

namespace prog {

// a program is composed of a list of commands
using program = std::vector<std::shared_ptr<const cmd::command>>;

inline bool ill_formed(const program& p) { return p.empty(); }

} // namespace prog

} // namespace bf