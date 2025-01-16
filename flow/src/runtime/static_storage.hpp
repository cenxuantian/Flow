#ifndef __FLOW_RUNTIME_STATIC_STORAGE_HPP__
#define __FLOW_RUNTIME_STATIC_STORAGE_HPP__

// the state machine of the language

#include "../types/types_def.hpp"

FLOW_NAMESPACE_BEGIN

#define WATER_DEFINE_NATIVE_FUNCTION(function_name)                            \
  static void function_name(list const &args)

#define WATER_ADD_NATIVE_FUNCTION_TO_OBJECT(function_name)                     \
  {#function_name, function(detail::native_functions::function_name)}

namespace detail {

namespace native_functions {

WATER_DEFINE_NATIVE_FUNCTION(print) {
  std::cout << "\033[0m\033[1;33m";
  for (auto &i : args) {
    std::cout << i.to_string() << ' ';
  }
  std::cout << "\n\033[0m";
}

WATER_DEFINE_NATIVE_FUNCTION(exit) {
  int exit_num = 0;
  if (args.size()) {
    exit_num = args[0].as<number>();
  }

  std::cout << "Program Exit with code (" << exit_num << ").\n";
  ::exit(exit_num);
}

} // namespace native_functions
} // namespace detail

static object static_storage = {
    // native functions
    WATER_ADD_NATIVE_FUNCTION_TO_OBJECT(print),
    WATER_ADD_NATIVE_FUNCTION_TO_OBJECT(exit),
};

FLOW_NAMESPACE_END

#endif
