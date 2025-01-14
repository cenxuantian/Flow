#ifndef __FLOW_UTILS_HPP__
#define __FLOW_UTILS_HPP__

// the state machine of the language

#include "./defs.hpp"

FLOW_NAMESPACE_BEGIN

namespace utils {

template <class Iterable, class Callable>
void remove_all(Iterable &s, Callable f) {
  s.erase(std::remove_if(s.begin(), s.end(), f), s.end());
}

// input string example : "a,b,c"
std::vector<std::string> split_args(std::string const &in_str, char spliter) {
  std::vector<std::string> ret;
  size_t l = 0;
  size_t r = 0;
  while (r < in_str.size()) {
    if (in_str[r] == spliter) {
      ret.emplace_back(in_str.substr(l, r - l));
      l = r + 1;
    }
    ++r;
  }
  ret.emplace_back(in_str.substr(l, r - l));
  return ret;
}

} // namespace utils

FLOW_NAMESPACE_END

#endif
