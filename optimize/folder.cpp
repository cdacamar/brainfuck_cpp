#include <optimize/folder.h>

#include <algorithm>
#include <type_traits>

#include <cmd/arithmetic.h>
#include <cmd/io.h>
#include <cmd/loop.h>
#include <cmd/shift.h>

namespace bf {

namespace optimize {

struct base_is_visitor : cmd::command::visitor {
  bool* result;
  base_is_visitor(bool& result) : result{ &result } { *this->result = false; }

  // stubs
  void operator()(const cmd::arithmetic&) const override { }
  void operator()(const cmd::io&)         const override { }
  void operator()(const cmd::loop&)       const override { }
  void operator()(const cmd::shift&)      const override { }
};
template <typename T, typename = std::enable_if_t<std::is_base_of<cmd::command, T>::value>>
struct is_visitor : base_is_visitor {
  using base_is_visitor::base_is_visitor;

  void operator()(const T&) const override { *result = true; }
};

template <typename T, typename = std::enable_if_t<std::is_base_of<cmd::command, T>::value>>
bool is(const cmd::command& c) {
  bool b;
  is_visitor<T> is_{ b };
  c.accept(is_);
  return *is_.result;
}

static std::shared_ptr<cmd::loop> fold_loop(const cmd::loop& l);

template <typename I>
struct folding_visitor : cmd::command::visitor {
  I* first;
  I* last;

  folding_visitor(I& first, I& last):
    first{ &first }, last{ &last } { }

  void operator()(const cmd::arithmetic&) const override {
    auto arith_end = std::find_if(*first, *last, [](const auto& c) {
      return !is<cmd::arithmetic>(*c);
    });
    // if the sequence is a single instruction, skip it, no folding can be done
    if (std::distance(*first, arith_end) <= 1) {
      ++*first;
      return;
    }
    // build the new arithmetic sequence
    std::int32_t total_amount = 0; // we're going to transform this to an unsigned at the end
    for (auto it = *first; it != arith_end; ++it) {
      const auto& arith = static_cast<const cmd::arithmetic&>(**it);
      total_amount += arith.type == cmd::arithmetic::arith_type::add ?
        static_cast<std::int32_t>(arith.amount) :
        -static_cast<std::int32_t>(arith.amount);
    }
    if (total_amount == 0) {
      // remove these altogether since there is no meaningful math happening
      *last = std::rotate(*first, arith_end, *last);
      return;
    }
    // remove others
    *last = std::rotate(*first, --arith_end, *last);
    if (total_amount < 0) {
      **first = std::make_shared<cmd::arithmetic>(cmd::arithmetic::arith_type::sub, static_cast<std::uint32_t>(-total_amount));
    }
    else {
      **first = std::make_shared<cmd::arithmetic>(cmd::arithmetic::arith_type::add, static_cast<std::uint32_t>(total_amount));
    }
    ++*first;
  }
  void operator()(const cmd::io&) const override {
    // no transformation
    ++*first;
  }
  void operator()(const cmd::loop& l) const override {
    auto loop_optimized = fold_loop(l);
    if (loop_optimized->statements.empty()) {
      // throw away this loop object
      if (*first + 1 != *last) {
        *last = std::rotate(*first, *first + 1, *last);
      }
      else {
        *last = *first;
      }
      return;
    }
    **first = std::move(loop_optimized);
    ++*first;
  }
  void operator()(const cmd::shift&) const override {
    auto shift_end = std::find_if(*first, *last, [](const auto& c) {
      return !is<cmd::shift>(*c);
    });
    // if the sequence is a single instruction, skip it, no folding can be done
    if (std::distance(*first, shift_end) <= 1) {
      ++*first;
      return;
    }
    // build the new shift sequence
    std::int32_t total_amount = 0; // we're going to transform this to an unsigned at the end
    for (auto it = *first; it != shift_end; ++it) {
      const auto& sh = static_cast<const cmd::shift&>(**it);
      total_amount += sh.type == cmd::shift::shift_type::right ?
        static_cast<std::int32_t>(sh.amount) :
        -static_cast<std::int32_t>(sh.amount);
    }
    if (total_amount == 0) {
      // remove these altogether since there is no meaningful math happening
      *last = std::rotate(*first, shift_end, *last);
      return;
    }
    // remove others
    *last = std::rotate(*first, --shift_end, *last);
    if (total_amount < 0) {
      **first = std::make_shared<cmd::shift>(cmd::shift::shift_type::left, static_cast<std::uint32_t>(-total_amount));
    }
    else {
      **first = std::make_shared<cmd::shift>(cmd::shift::shift_type::right, static_cast<std::uint32_t>(total_amount));
    }
    ++*first;
  }
};

static std::shared_ptr<cmd::loop> fold_loop(const cmd::loop& l) {
  auto cmds  = l.statements;
  auto first = std::begin(cmds);
  auto last  = std::end(cmds);
  while (first != last) {
    folding_visitor<cmd::loop::cmd_list_t::iterator> fv{ first, last };
    (*first)->accept(fv);
  }
  cmds.erase(last, std::end(cmds));
  return std::make_shared<cmd::loop>(std::move(cmds));
}

void fold(prog::program& p) {
  // step through program and see what expressions we can collapse into a single routine
  auto first = std::begin(p);
  auto last  = std::end(p);
  while (first != last) {
    folding_visitor<prog::program::iterator> fv{ first, last };
    (*first)->accept(fv); // this will update [first, last)
  }
  p.erase(last, std::end(p)); // remove entries that were eliminated
}

} // namespace optimize

} // namespace bf