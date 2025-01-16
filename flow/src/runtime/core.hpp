#ifndef __FLOW_RUNTIME_CORE_H__
#define __FLOW_RUNTIME_CORE_H__

#include "../defs.hpp"
#include "../script_stream.hpp"
#include "../utils.hpp"
#include "./context.hpp"

FLOW_NAMESPACE_BEGIN

#define FLOW_RUNNER_CHECK_END()                                                \
  {                                                                            \
    script_.seek(script_.first_valid());                                       \
    if (script_.eof()) {                                                       \
      printf("Note: end of file\n");                                           \
      return;                                                                  \
    }                                                                          \
  }

#define FLOW_RUNNER_RISE_ERROR_IF_END(msg)                                     \
  {                                                                            \
    script_.seek(script_.first_valid());                                       \
    if (script_.eof()) {                                                       \
      ERROR_EXIT(msg);                                                         \
      return;                                                                  \
    }                                                                          \
  }

#define FLOW_RUNNER_CALL_FUNC(funcname, ...)                                   \
  funcname(__FUNCTION__, ##__VA_ARGS__)

#define FLOW_RUNNER_STATE_START()                                              \
  [[maybe_unused]] __START_LABLE__:                                            \
  FLOW_RUNNER_CHECK_END()

#define FLOW_RUNNER_GOTO_PRE_STATE() return

#define FLOW_RUNNER_GOTO_STATE(state_name, ...)                                \
  state_name(__FUNCTION__, ##__VA_ARGS__)

#define FLOW_RUNNER_STATE_FUNC_DEF(funcname, ...)                              \
  void funcname(std::string const &__PRE_STATE__, ##__VA_ARGS__)

namespace detail {

struct expr_stack_data {
  value r_value = value();
  value l_value = value();
  std::string oper = keywords::invalid_operator;
};

class runner {
private:
  void call_oper(expr_stack_data &in) {
    if (in.oper == keywords::invalid_operator) {
      // do nothing
    } else if (in.oper == keywords::assign_operator) {
      in.l_value.store(in.r_value.copy());
    } else if (in.oper == "+") {
      value temp = in.l_value.copy();
      temp.add(in.r_value);
      in.l_value.swap(temp);
    } else if (in.oper == "-") {
      value temp = in.l_value.copy();
      temp.minus(in.r_value);
      in.l_value.swap(temp);
    } else if (in.oper == "*") {
      value temp = in.l_value.copy();
      temp.multiply(in.r_value);
      in.l_value.swap(temp);
    } else if (in.oper == "/") {
      value temp = in.l_value.copy();
      temp.divide(in.r_value);
      in.l_value.swap(temp);
    } else if (in.oper == "%") {
      value temp = in.l_value.copy();
      temp.mold(in.r_value);
      in.l_value.swap(temp);
    } else if (in.oper == "#") { // all func
      call_fn(in);
      in.r_value.store(std::move(in.l_value.as<function>().get_output()));
      in.l_value.swap(in.r_value);
    }
    // clear oper and r_value
    in.oper = keywords::invalid_operator;
    value temp;
    in.r_value.swap(temp);
  }

  void call_fn(expr_stack_data &in) {
    function const &func = in.l_value.as<function>();
    list &args = in.r_value.as<list>();
    switch (func.type_) {
    case function::types::CPP_FN: {
      func.cpp_fn(args);
      break;
    }
    case function::types::SCRIPT_FN: {
      /* code */
      break;
    }
    default:
      break;
    }
  }

  FLOW_RUNNER_STATE_FUNC_DEF(normal_state) {
    FLOW_RUNNER_STATE_START();
    char current = script_.current_char();
    switch (current) {
    case ';': {
      FLOW_RUNNER_CHECK_END();
      script_.seek(script_.next_valid());
      FLOW_RUNNER_CHECK_END();
      return;
    }
    default: {
      std::optional<expr_stack_data> opt{};
      FLOW_RUNNER_GOTO_STATE(parse_expr, opt, ';');
      return;
    }
    }
  }

  FLOW_RUNNER_STATE_FUNC_DEF(parse_list, std::string const &in, list &out) {
    script_stream temp = in;
    std::swap(script_, temp);

    FLOW_RUNNER_STATE_START();
    FLOW_RUNNER_RISE_ERROR_IF_END("Syntax error");

    while (!script_.eof()) {
      FLOW_RUNNER_CHECK_END();
      char current = script_.current_char();
      if (keywords::is_valid(current) && current != ',') {
        std::optional<expr_stack_data> opt;
        FLOW_RUNNER_GOTO_STATE(parse_expr, opt, ',');
        out.emplace_back(std::move(opt.value().l_value));
      } else {
        script_.seek(script_.next_valid());
      }
    }
    std::swap(script_, temp);
  }

  FLOW_RUNNER_STATE_FUNC_DEF(parse_expr, std::optional<expr_stack_data> &in,
                             char end) {
  [[maybe_unused]] __START_LABLE__:

    script_.seek(script_.first_valid());
    if (script_.eof()) {
      if (in.has_value()) {
        call_oper(in.value());
        FLOW_RUNNER_GOTO_PRE_STATE();
      } else {
        printf("Warning empty expr\n");
        // warning, empty expression
      }
    }

    char current = script_.current_char();

    if (current == '(') { // function or sub expr
      if (in.has_value()) {
        if (in.value().l_value.type() == types::FUNCTION &&
            in.value().oper == keywords::invalid_operator) { // function
          std::string args_str = script_.read_full_bracket();
          list args;
          if (args_str.size()) {
            FLOW_RUNNER_GOTO_STATE(parse_list, args_str, args);
          }
          in.value().r_value.store(std::move(args));
          in.value().oper = "#";
          call_oper(in.value());
        }
      } else {
      }
    } else if (current == '=' || current == '-' || current == '+' ||
               current == '/' || current == '*') { // value assignment
      if (!in.has_value()) {
        if (current == '-') { // to catch statement like -1
          in.emplace();
          in.value().l_value.store(0);
        } else {
          ERROR_EXIT("Syntax Error: No left value in front of an operator.");
        }
      }
      if (in.value().oper !=
          keywords::invalid_operator) { // already got a different operator
        ERROR_EXIT("Syntax Error: duplicated operator.");
      } else { // add operator
        char temp[2] = {current, 0};
        in.value().oper = temp;
        script_.seek(script_.next_valid());
        FLOW_RUNNER_GOTO_STATE(parse_expr, in, end);
      }
    } else if (current == end) // end of this expr
    {
      if (in.has_value()) {
        call_oper(in.value());
        FLOW_RUNNER_GOTO_PRE_STATE();
      } else {
        // warning, empty expression
      }

      script_.seek(script_.next_valid());
      FLOW_RUNNER_GOTO_PRE_STATE();
    } else if (current == '#') // function
    {
    } else if (current == '?') {
    } else {
      if (!in.has_value()) { // parse l_value
        in.emplace();
        FLOW_RUNNER_GOTO_STATE(parse_value, in.value().l_value); // will seek
        FLOW_RUNNER_GOTO_STATE(parse_expr, in, end);
      } else { // parse r_value
        ERROR_EXIT_IF(keywords::invalid_operator == in.value().oper,
                      "Expecting a `;`");
        std::optional<expr_stack_data> next_in{}; // empty option
        FLOW_RUNNER_GOTO_STATE(parse_expr, next_in, end);
        in.value().r_value.store(std::move(next_in.value().l_value));
        call_oper(in.value());
        FLOW_RUNNER_GOTO_STATE(parse_expr, in, end);
      }
    }
  }

  FLOW_RUNNER_STATE_FUNC_DEF(parse_value, value &out) {
    FLOW_RUNNER_STATE_START();
    char current = script_.current_char();

    if (keywords::is_number(current)) {
      out.store(script_.read_full_number());
    } else if (keywords::is_string_begin_end(current)) {
      out.store(script_.read_full_string());
    } else if (current == ':') { // parse a function
      script_.seek(script_.next_valid());
      out.store(function(script_.read_full_bracket()));
    } else { // get a symbol
      std::string symbol_name = script_.read_full_word();
      out.store(ctx_.get_symbol_value(symbol_name).ref());
    }
  }

  script_stream script_;
  std::vector<std::string> args_;
  context &ctx_;

public:
  template <class StringLike>
  runner(context &ctx, StringLike &&string_like,
         std::vector<std::string> const &args)
      : script_(string_like), args_(args), ctx_(ctx) {
    while (!script_.eof()) {
      FLOW_RUNNER_CALL_FUNC(normal_state);
    }
  }
};

} // namespace detail

class core {

public:
  template <class StringLike>
  static void exec(context &ctx, StringLike &&string_like,
                   std::vector<std::string> const &args = {}) {
    detail::runner runner(ctx, std::forward<StringLike &&>(string_like), args);
  }
};

FLOW_NAMESPACE_END
#endif
