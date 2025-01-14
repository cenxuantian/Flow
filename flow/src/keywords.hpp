#ifndef __FLOW_KEYWORDS_HPP__
#define __FLOW_KEYWORDS_HPP__

#include "defs.hpp"

FLOW_NAMESPACE_BEGIN

struct keywords {
public:
  static inline const std::set<char> invalid_chars = {' ', '\n', '\t', '\r'};
  static inline const std::set<char> string_begin_end = {'\'', '"', '`'};
  static inline const std::set<char> valid_chars_in_name = {'_'};
  static inline const std::set<std::string> key_words = {};
  static inline const std::string invalid_operator = "";
  static inline const std::string assign_operator = "=";
  static inline const std::set<std::string> operators = {
      "+",  "-", "*", "/",  "=",  "|",  "||", "&",
      "&&", "%", "^", "<=", "<<", ">=", ">>", "!",
      "#", // call function
      "~",  "^",
  };

public:
  static inline bool is_string_begin_end(char c) {
    return string_begin_end.count(c);
  }
  static inline bool is_valid(char c) { return !invalid_chars.count(c); }
  static inline bool is_number(char c) { return c >= '0' && c <= '9'; }
  static inline bool is_word(char c) {
    return is_number(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           valid_chars_in_name.count(c);
  }
};

FLOW_NAMESPACE_END
#endif
