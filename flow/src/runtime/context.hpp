#ifndef __FLOW_RUNTIME_CONTEXT_H__
#define __FLOW_RUNTIME_CONTEXT_H__

// the state machine of the language

#include "../types/types_def.hpp"

FLOW_NAMESPACE_BEGIN

struct stack_data {
  value local_storage = object{};
  size_t start_line = 0;
  std::string filename = "";
};

static object static_storage = {
    {"print", function([](list const &args) {
       std::cout << "\033[0m\033[1;33m";
       for (auto &i : args) {
         std::cout << i.to_string() << ' ';
       }
       std::cout << "\n\033[0m";
     })},
};

class context {

private:
  value global_storage_; // global storage for this contaxt
  std::stack<stack_data> stacks_;

  object &local_storage() { return stacks_.top().local_storage.as<object>(); }

  object &global_storage() { return global_storage_.as<object>(); }

public:
  context() : global_storage_(object(static_storage)), stacks_() {
    stack_data s_data;
    stacks_.push(s_data);
  }

  void push_stack(stack_data const &s) { stacks_.push(s); }
  void pop_stack() { stacks_.pop(); }

  void init_local_variable(std::string const &name, value &&val) {
    local_storage()[name].store(std::forward<value &&>(val));
  }

  void init_global_variable(std::string const &name, value &&val) {
    global_storage()[name].store(std::forward<value &&>(val));
  }

  value &get_symbol_value(std::string const &name) {
    if (name == "__local__") {
      return stacks_.top().local_storage;
    } else if (name == "__global__") {
      return global_storage_;
    }
    if (local_storage().count(name)) {
      return local_storage()[name];
    } else if (global_storage().count(name)) {
      return global_storage()[name];
    } else {
      local_storage().emplace(name, value());
      return local_storage()[name];
    }
  }

  void set_symbol_value(std::string const &name, value &&val) {
    if (local_storage().count(name)) {
      local_storage()[name].store(std::forward<value &&>(val));
    } else if (global_storage().count(name)) {
      global_storage()[name].store(std::forward<value &&>(val));
    } else {
      ERROR_EXIT("Undefined symbol\n");
    }
  }

  stack_data &current_stack() { return stacks_.top(); }
};

FLOW_NAMESPACE_END

#endif
