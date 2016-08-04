#include <code_gen/code_gen.h>

#include <sstream>

// commands
#include <cmd/arithmetic.h>
#include <cmd/io.h>
#include <cmd/loop.h>
#include <cmd/shift.h>

namespace bf {

namespace code_gen {

js_generator::js_generator(prog::program p, const std::string& outfile):
  p_{ std::move(p) },
  file_{ outfile } {
  if (!file_) throw std::ofstream::failure{ "could not open file for writing" };
}

void js_generator::emit() {
  emit_html_();
}

static std::string cmd_to_js(const cmd::command& c) {
  struct loop_depth_visitor : cmd::command::visitor {
    std::size_t* depth;
    loop_depth_visitor(std::size_t& depth) : depth{ &depth } { *this->depth = 0; }

    void operator()(const cmd::arithmetic&) const override {
      ++*depth;
    }
    void operator()(const cmd::io&) const override {
      ++*depth;
    }
    void operator()(const cmd::shift&) const override {
      ++*depth;
    }
    void operator()(const cmd::loop& l) const override {
      ++*depth; // for the check
      for (const auto& stmt : l.statements) {
        stmt->accept(*this);
      }
      ++*depth; // for the jump
    }
  };

  std::stringstream ss;
  struct js_generate_visitor : cmd::command::visitor {
    std::stringstream* ss;
    js_generate_visitor(std::stringstream& ss) : ss{ &ss } { }

    void operator()(const cmd::arithmetic& a) const override {
      auto amount = (a.type == cmd::arithmetic::arith_type::add? static_cast<std::int32_t>(a.amount) : -static_cast<std::int32_t>(a.amount));
      *ss <<
        "function() { value_arr_[cur_] += " << amount << "; SP_ += 1; }";
    }
    void operator()(const cmd::io& io) const override {
      if (io.type == cmd::io::io_type::in) {
        *ss << "self_.get_char";
      }
      else {
        *ss << "self_.put_char";
      }
    }
    void operator()(const cmd::shift& s) const override {
      auto amount = (s.type == cmd::shift::shift_type::right ? static_cast<std::int32_t>(s.amount) : -static_cast<std::int32_t>(s.amount));
      *ss <<
        "function() { if (cur_ + " << amount << " < 0) { self_.seg_fault(); return; } " <<
        "cur_ += " << amount << "; " <<
        "if (value_arr_[cur_] === undefined) { value_arr_[cur_] = 0; } " <<
        "SP_ += 1; }";
    }
    void operator()(const cmd::loop& l) const override {
      std::size_t depth;
      loop_depth_visitor depth_visitor{ depth };
      l.accept(depth_visitor);

      // condition
      *ss << "function() { if (value_arr_[cur_] === 0) { " <<
        "SP_ += " << *depth_visitor.depth << "; }" <<
        "else { SP_ += 1; } }";

      // body
      for (const auto& stmt : l.statements) {
        *ss << ',' << cmd_to_js(*stmt);
      }

      // jump
      *ss << ",function() { SP_ -= " << *depth_visitor.depth - 1 << "; }";
    }
  } gen_visitor{ ss };
  c.accept(gen_visitor);

  return gen_visitor.ss->str();
}

void js_generator::emit_code_() {
  const char* to_cmds =
R";;(var Program = function() {
  self_      = undefined;
  io_mgr_    = undefined;
  commands_  = [];
  SP_        = 0;
  SP_save_   = 0;
  value_arr_ = [];
  cur_      = 0;

  return {
    init: function(io_mgr) {
      self_ = this;

      value_arr_[cur_] = 0;

      io_mgr_   = io_mgr;
      commands_ = [);;";

  file_ << to_cmds;

  std::string comma = "";
  for (const auto& cmd : p_) {
    file_ << comma << cmd_to_js(*cmd);
    comma = ",";
  }

  const char* rest =
R";;(];
    },
    get_char: function() {
      var input = io_mgr_.poll_char();
      if (input !== undefined) {
        value_arr_[cur_] = input.charCodeAt(0);
        SP_ = SP_save_ + 1;
        self_.process_command();
      }
      else {
        SP_ = commands_.length;
        setTimeout(function() { self_.get_char() }, 500);
      }
    },
    put_char: function() {
      io_mgr_.put_char(String.fromCharCode(value_arr_[cur_]));
      SP_ += 1;
    },
    seg_fault: function() {
      io_mgr_.put_line('segmentation fault');
      SP_ = commands_.length;
    },
    process_command: function() {
      while (SP_ < commands_.length) {
        var cmd = commands_[SP_];
        SP_save_ = SP_;
        cmd.apply(self_);
      }
    },
    exe: function() {
      self_.process_command();
    }
  };
};
);;";
  file_ << rest;
}

void js_generator::emit_css_() {
  const char* css_ =
R";;(<style>
.input {
  display: inline-block;
  width: 49%;
  height: 500px;
}
.output {
  display: inline-block;
  width: 49%;
  height: 500px;
}
label {
  display: block;
}
textarea {
  background-color: black;
  color: white;
  border: none;
  width: 95%;
  height: 95%;
}
</style>
);;";
  file_ << css_;
}

void js_generator::emit_html_() {
  const char* partial_head =
R";;(<!DOCTYPE html>
<html>
);;";
  file_ << partial_head;
  emit_css_();

  const char* to_prog =
R";;(<body>
<div class='input'>
<label for='in'>Input</label>
<textarea id='in'></textarea>
</div>

<div class='output'>
<label for='out'>Output</label>
<textarea id='out'></textarea>
</div>

<script>
);;";
  file_ << to_prog;

  emit_code_();
  file_ << "</script>\n";

  emit_base_js_();

  const char* end =
R";;(</body>
</html>);;";

  file_ << end;
}

void js_generator::emit_base_js_() {
  const char* base_js =
R";;(<script>
window.addEventListener('load', function() {
  var input_text  = document.getElementById('in');
  var output_text = document.getElementById('out');

  // create input manager with the textarea
  this.io_mgr = new IO_mgr();
  this.io_mgr.init(input_text, output_text);

  /*
  var check_input = function() {
    var input = io_mgr.poll_char();
    if (input !== undefined) {
      var out = document.getElementById('out');
      out.value += "\nNew value: " + input;
    }
    setTimeout(check_input, 500);
  }
  check_input();
  */

  this.prog = new Program();
  this.prog.init(this.io_mgr);
  this.prog.exe();
});

var IO_mgr = function() {
  var key_id_ = {
    ENTER: 13
  };
  var input_  = undefined;
  var output_ = undefined;
  var lines_  = [];

  return {
    init: function(input, output) {
      input_  = input;
      output_ = output;

      input_.addEventListener('keydown', this.handle_keydown);
      input_.addEventListener('keyup', this.handle_keyup);
    },
    poll_line: function() {
      if (lines_.length > 0) return lines_.shift();
      return undefined;
    },
    poll_char: function() {
      if (lines_.length > 0) {
        if (lines_[0].length > 0) {
          var c = lines_[0][0];
          lines_[0] = lines_[0].slice(1, lines_[0].length);
          return c;
        }
        lines_.shift();
        return '\n';
      }
      return undefined;
    },
    put_char: function(c) {
      output_.value += c;
    },
    put_line: function(line) {
      output_.value += line + '\n';
    },
    handle_keydown: function(e) {
      if (e.keyCode === key_id_.ENTER) {
        e.preventDefault();
      }
    },
    handle_keyup: function(e) {
      if (e.keyCode === key_id_.ENTER) {
        lines_.push(input_.value.split('\n').pop());
        input_.value += '\n';
      }
    }
  };
};
</script>
);;";

  file_ << base_js;
}

} // namespace code_gen

} // namespace bf