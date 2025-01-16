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

/// @brief This class read and analyze the input scripts
class runner {
private:
  /// @brief Execute the operator.
  /// Use both l_value and r_value as inputs.
  /// Store the result in l_value.
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

  /// @brief Execute an Flow function
  /// Use both l_value and r_value as inputs.
  /// Store the result in l_value.
  void call_fn(expr_stack_data &in) {
    function const &func = in.l_value.as<function>();
    list &args = in.r_value.as<list>();
    switch (func.type_) {
    case function::types::CPP_FN: {
      func.cpp_fn(args);
      break;
    }
    case function::types::SCRIPT_FN: {
      stack_data func_state;
      func_state.local_storage.as<object>().emplace("__args__",in.r_value.ref());
      func_state.local_storage.as<object>().emplace("__ret__",value(10));
      ctx_.push_stack(func_state);
      std::string & inner_text = in.l_value.as<function>().inner_text;
      [[maybe_unused]]runner sub_runner(ctx_,inner_text,{});
      [[maybe_unused]]auto& tt = func_state.local_storage.as<object>()["@ret"];
      in.l_value.as<function>().out_.store(std::move(tt));
      ctx_.pop_stack();
      break;
    }
    default:
      break;
    }
  }

  /// @brief The first state, all other states will be routed from here.
  /// When all other states end, will redirect to this state.
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

  /// @brief Parse the functions' input parameters list or an initialize-list
  /// @param in The input string with out braces. For example, if an input
  /// parameter of a function is `(1,2,3,4)`, the string `1,2,3,4` should be
  /// passed into this state as the `in` param.
  /// @param out return the out list
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

  /// @brief One of the most important state, parse any kind of statement.
  /// @param in the input value struct, if no value is passing by previous
  /// state, it should be std::optional<expr_stack_data>{}.
  /// (Note:flow::types::UNDEFINED is also regarded as value).
  /// @param end the end flag of this expr, for functions, it is `,`, for
  /// others, it it `;`.
  FLOW_RUNNER_STATE_FUNC_DEF(parse_expr, std::optional<expr_stack_data> &in,
                             char end) {

    // Check if reach the end of the expr
    script_.seek(script_.first_valid());
    if (script_.eof()) {
      if (in.has_value()) {
        call_oper(in.value());
        FLOW_RUNNER_GOTO_PRE_STATE();
      } else {
        // warning, empty expression
        printf("Warning: empty expr\n");
        FLOW_RUNNER_GOTO_PRE_STATE();
      }
    }

    char current = script_.current_char();

    // Parse `function param list` or `sub expr`
    if (current == '(') {
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
    }

    // Parse and excuse operator
    else if (current == '=' || current == '-' || current == '+' ||
             current == '/' || current == '*') {
      if (!in.has_value()) {
        // if there is no l_value, then: to catch statement like -1
        if (current == '-') {
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
    }

    // Parse end of this expr
    else if (current == end) {
      if (in.has_value()) {
        call_oper(in.value());
        FLOW_RUNNER_GOTO_PRE_STATE();
      } else {
        // warning, empty expression
      }

      script_.seek(script_.next_valid());
      FLOW_RUNNER_GOTO_PRE_STATE();
    }

    // Parse conditional statement
    else if (current == '?') {
    }

    // Parse other statement
    else {
      if (!in.has_value()) { // parse l_value
        in.emplace();
        FLOW_RUNNER_GOTO_STATE(parse_value, in.value().l_value); // will seek
        FLOW_RUNNER_GOTO_STATE(parse_expr, in, end);
      } else { // parse r_value via parsing sub_expr
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

  /// @brief Parse an value.
  /// An value can be:
  /// - string: e.g., "hello world" 'hello world' `hello world`
  /// - number: e.g., 1 -1
  /// - symbol: e.g., __global__
  /// - function: e.g., #{...}
  /// - function parames, @ret @1 @2 @3 ...
  /// - list: e.g. [1,2,3,4,5,...]
  /// - object: { name: value }
  FLOW_RUNNER_STATE_FUNC_DEF(parse_value, value &out) {
    FLOW_RUNNER_STATE_START();
    char current = script_.current_char();

    if (keywords::is_number(current)) { // parse number
      out.store(script_.read_full_number());
    } else if (keywords::is_string_begin_end(current)) { // parse 
      out.store(script_.read_full_string());
    } 
    else if(current == '#'){ // parse function
      script_.seek(script_.next_valid());
      FLOW_RUNNER_RISE_ERROR_IF_END("Syntax error: expecting `{`");
      out.store(function(script_.read_full_bracket()));
    }
    else { // get a symbol
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
