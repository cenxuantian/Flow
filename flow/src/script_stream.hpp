#ifndef __FLOW_STRING_HPP__
#define __FLOW_STRING_HPP__

#include "defs.hpp"
#include "keywords.hpp"

FLOW_NAMESPACE_BEGIN

class script_stream {
private:
  std::string str;
  size_t pos;

  size_t next_valid_string_end_impl(size_t start_pos) {
    char start_with = at(start_pos);
    assert(keywords::is_string_begin_end(start_with));
    for (size_t i = start_pos + 1; i < str.size(); ++i) {
      if (str[i] == start_with && str[i - 1] != '\\') {
        return i + 1;
      }
    }
    return str.size();
  }

public:
  template <class T> script_stream(T &&c_str) : str(c_str), pos(0) {}

  char current_char() const { return str[pos]; }

  const char *c_str() const { return str.data() + pos; }

  size_t eof() const { return pos == str.size(); }

  size_t size() const { return str.size(); }

  void seek(size_t target_pos) { pos = target_pos; }

  size_t tell() const { return pos; }

  size_t first_valid() const {
    for (size_t i = pos; i < str.size(); ++i) {
      if (keywords::is_valid(str[i])) {
        return i;
      }
    }
    return str.size();
  }

  // should add test
  size_t next_valid() const {
    for (size_t i = pos + 1; i < str.size(); ++i) {
      if (keywords::is_valid(str[i])) {
        return i;
      }
    }
    return str.size();
  }

  // should add test
  // returns next index of the last position of this word
  size_t this_word_end() const {
    // assert(keywords::is_word(current_char()));
    for (size_t i = pos + 1; i < str.size(); ++i) {
      if (!keywords::is_word(str[i])) {
        return i;
      }
    }
    return str.size();
  }

  // will seek
  std::string read_full_word() {
    auto end_pos = this_word_end();
    auto start_pos = pos;
    seek(end_pos);
    return str.substr(start_pos, end_pos - start_pos);
  }

  // will seek
  double read_full_number() {
    auto end_pos = next_valid_number_end();
    auto start_pos = pos;
    seek(end_pos);
    return std::stod(str.substr(start_pos, end_pos - start_pos));
  }

  std::string read_full_string() {
    auto end_pos = next_valid_string_end();
    auto start_pos = pos;
    seek(end_pos);
    return str.substr(start_pos + 1, (end_pos - start_pos) - 2);
  }

  std::string read_full_bracket() {
    auto end_pos = next_valid_bracket_end();
    auto start_pos = pos;
    seek(end_pos);
    return str.substr(start_pos + 1, (end_pos - start_pos) - 2);
  }

  std::string exclusive_substr(size_t begin, size_t end) {
    return str.substr(begin, end - begin);
  }

  char at(size_t position) {
    assert(position < str.size());
    return str[position];
  }

  // should add test
  // returns next position of `'`
  size_t next_valid_string_end() { return next_valid_string_end_impl(pos); }

  // should add test
  // returns next position of `}`
  size_t next_valid_bracket_end() {
    char end_with = '}';
    char start_with = current_char();
    if (start_with == '[') {
      end_with = ']';
    } else if (start_with == '(') {
      end_with = ')';
    }
    std::stack<size_t> temp;
    temp.push(pos);
    for (size_t i = pos + 1; i < str.size(); ++i) {
      if (keywords::is_string_begin_end(str[i])) {
        i = next_valid_string_end_impl(i) - 1;
      } else if (str[i] == end_with) {
        temp.pop();
        if (temp.empty()) {
          return i + 1;
        }
      } else if (str[i] == start_with) {
        temp.push(i);
      }
    }
    return str.size();
  }
  size_t next_valid_number_end() {
    for (size_t i = pos + 1; i < str.size(); ++i) {
      if (!(keywords::is_number(str[i]) || str[i] == '.') &&
          str[i - 1] != '.') {
        return i;
      }
    }
    return str.size();
  }
  size_t next_command_end() {
    for (size_t i = pos + 1; i < str.size(); ++i) {
      if (str[i] == ';') {
        return i;
      }
    }
    return str.size();
  }
};

FLOW_NAMESPACE_END

#endif
