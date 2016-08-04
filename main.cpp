#include <cstring>

#include <algorithm>
#include <iostream>
#include <string>
#include <tuple>

#include <code_gen/code_gen.h>
#include <lex/lexer.h>
#include <optimize/optimizer.h>
#include <parse/parser.h>

void dump_tokens(bf::lex::lexer& l);
void dump_commands(const bf::prog::program& p);

int main(int argc, const char* argv[]) {
  enum class mode_t {
    dump_tokens,
    dump_commands,
    compile
  } mode = mode_t::compile;
  bool optimize_ = false;

  const char* filename = nullptr;
  std::string outfile  = "a.html";
  for (int i = 1; i < argc; ++i) {
    const char* arg = argv[i];

    if (*arg != '-') {
      filename = arg;
    }
    else if (std::strcmp(arg, "--dump-tokens") == 0) {
      mode = mode_t::dump_tokens;
    }
    else if (std::strcmp(arg, "--dump-commands") == 0) {
      mode = mode_t::dump_commands;
    }
    else if (std::strcmp(arg, "--optimize") == 0) {
      optimize_ = true;
    }
    else if (std::strcmp(arg, "-o") == 0 && i + 1 < argc) {
      outfile = argv[i + 1];
      ++i;
    }
    else {
      std::cerr << "unexpected argument: " << arg << '\n';
      return 1;
    }
  }

  if (!filename) {
    std::cerr << "no input\n";
    return 1;
  }

  using namespace bf;

  try {
    lex::lexer l{ filename };

    if (mode == mode_t::dump_tokens) {
      dump_tokens(l);
      return 0;
    }

    parse::parser p{ l };

    auto program = p.parse();
    if (prog::ill_formed(program)) {
      for (const auto& e : p.errors()) {
        std::cerr << e << '\n';
      }
      return 1;
    }

    if (optimize_) {
      optimize::optimize(program);
    }

    if (mode == mode_t::dump_commands) {
      dump_commands(program);
      return 0;
    }

    if (mode == mode_t::compile) {
      try {
        code_gen::js_generator gen{ std::move(program), outfile };
        gen.emit();
      }
      catch (const std::exception& e) {
        std::cerr << "failed to open file: " << outfile << ": " << e.what();
        return 1;
      }
    }
  }
  catch (const std::exception& e) {
    std::cerr << "failed to open file: " << filename << ": " << e.what();
    return 1;
  }
}

void dump_tokens(bf::lex::lexer& l) {
  auto tok = l.next_token();

  using namespace std::literals;
  std::string headers[] = {
    "Token "s,
    "Line "s,
    "Column"s
  };
  std::vector<std::tuple<std::string, std::string, std::string>> tokens;
  std::size_t longest[] = {
    headers[0].size(),
    headers[1].size(),
    headers[2].size()
  };

  while (tok.type != bf::lex::token_t::eof) {
    tokens.emplace_back("'"s + bf::lex::tok_to_c(tok.type) + "'"s, std::to_string(tok.pos.line), std::to_string(tok.pos.col));
    tok = l.next_token();

    longest[0] = std::max(longest[0], std::get<0>(tokens.back()).size());
    longest[1] = std::max(longest[1], std::get<1>(tokens.back()).size());
    longest[2] = std::max(longest[2], std::get<2>(tokens.back()).size());
  }

  // output
  auto sp_fill = [](const std::string& str, std::size_t rjust) {
    std::cout << str;
    rjust -= str.size();
    while (rjust-- > 0) std::cout << ' ';
  };
  // headers
  sp_fill(headers[0], longest[0]);
  sp_fill(headers[1], longest[1]);
  sp_fill(headers[2], longest[2]);
  std::cout.put('\n');

  // tokens
  for (const auto& t : tokens) {
    sp_fill(std::get<0>(t), longest[0]);
    sp_fill(std::get<1>(t), longest[1]);
    sp_fill(std::get<2>(t), longest[2]);
    std::cout.put('\n');
  }
}

#include <cmd/arithmetic.h>
#include <cmd/io.h>
#include <cmd/loop.h>
#include <cmd/shift.h>

void dump_command(const bf::cmd::command& c, std::size_t indent) {
  for (std::size_t i = 0; i < indent; ++i) std::cout << ' ';

  struct print_visitor : bf::cmd::command::visitor {
    std::size_t indent;
    print_visitor(std::size_t indent) : indent{ indent } { }

    void operator()(const bf::cmd::arithmetic& a) const override {
      std::cout << "arithmetic: type{" << (a.type == bf::cmd::arithmetic::arith_type::add ? "add" : "subtract") << "} amount{" << a.amount << "}";
    }
    void operator()(const bf::cmd::io& io) const override {
      std::cout << "I/O: type{" << (io.type == bf::cmd::io::io_type::in ? "input" : "output") << "}";
    }
    void operator()(const bf::cmd::loop& l) const override {
      std::cout << "loop: {\n";
      for (const auto& stmt : l.statements) {
        dump_command(*stmt, indent + 2);
        std::cout.put('\n');
      }
      for (std::size_t i = 0; i < indent; ++i) std::cout << ' ';
      std::cout << "}";
    }
    void operator()(const bf::cmd::shift& s) const override {
      std::cout << "shift: type{" << (s.type == bf::cmd::shift::shift_type::left ? "left" : "right") << "} amount{" << s.amount << "}";
    }
  } pv{ indent };
  c.accept(pv);
}

void dump_commands(const bf::prog::program& p) {
  for (const auto& c : p) {
    dump_command(*c, 0);
    std::cout.put('\n');
  }
}