#ifndef __FLOW_TYPES_FUNCTION_CPP__
#define __FLOW_TYPES_FUNCTION_CPP__

// the state machine of the language

#include "types_def.hpp"

FLOW_NAMESPACE_BEGIN

bool function::operator==(function const &other) const {
  return inner_text == other.inner_text;
}

function::function() : type_(types::SCRIPT_FN), inner_text(""), cpp_fn() {}

function::function(std::string const &func_str)
    : type_(types::SCRIPT_FN), inner_text(func_str), cpp_fn() {}

function::function(std::function<void(list const &)> const &in_fn)
    : type_(types::CPP_FN), inner_text(""), cpp_fn(in_fn) {}

value &&function::get_output() const {
  return std::move(const_cast<value &>(out_));
}

FLOW_NAMESPACE_END

#endif
