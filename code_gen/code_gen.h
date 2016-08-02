#pragma once
#include <fstream>

#include <cmd/program.h>

namespace bf {

namespace code_gen {

class js_generator {
  prog::program p_;
  std::ofstream file_;

  void emit_css_();
  void emit_html_();
  void emit_base_js_();
  void emit_code_();
public:
  js_generator(prog::program p, const std::string& outfile);

  void emit();
};

}

}