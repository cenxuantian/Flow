#ifndef __FLOW_TYPES_VALUE_CPP__
#define __FLOW_TYPES_VALUE_CPP__

// the state machine of the language

#include "types_def.hpp"

FLOW_NAMESPACE_BEGIN

value::value()
    : type_(types::UNDEFINED), data(nullptr), is_ref(false), ref_tar_(nullptr) {
}

value::value(value &&other) noexcept
    : type_(other.type_), data(other.data), is_ref(other.is_ref),
      ref_tar_(other.ref_tar_) {
  other.leak();
}

value::value(value const &other)
    : type_(other.type_), data(nullptr), is_ref(false), ref_tar_(nullptr) {
  switch (type_) {
  case types::OBJECT:
    data = new object(other.as<object>());
    break;
  case types::LIST:
    data = new list(other.as<list>());
    break;
  case types::STRING:
    data = new string(other.as<string>());
    break;
  case types::FUNCTION:
    data = new function(other.as<function>());
    break;
  case types::NUMBER:
    data = new number(other.as<number>());
    break;
  case types::UNDEFINED:
  default:
    break;
  }
}

value::~value() { release(); }

void value::release() {
  if (is_ref) {
    leak();
    return;
  }

  switch (type_) {
  case types::OBJECT:
    delete &as<object>();
    data = nullptr;
    type_ = types::UNDEFINED;
    break;

  case types::LIST:
    delete &as<list>();
    data = nullptr;
    type_ = types::UNDEFINED;
    break;
  case types::STRING:
    delete &as<string>();
    data = nullptr;
    type_ = types::UNDEFINED;
    break;
  case types::FUNCTION:
    delete &as<function>();
    data = nullptr;
    type_ = types::UNDEFINED;
    break;
  case types::NUMBER:
    delete &as<number>();
    data = nullptr;
    type_ = types::UNDEFINED;
  case types::UNDEFINED:
  default:
    break;
  }
}

void value::leak() {
  type_ = types::UNDEFINED;
  data = nullptr;
  is_ref = false;
  ref_tar_ = nullptr;
}
types value::type() const { return type_; }
void value::store(value &&other) {
  if (is_ref) {
    this->ref_tar_->store(std::forward<value &&>(other));
    this->data = ref_tar_->data;
    this->type_ = ref_tar_->type_;
    other.leak();
  } else {
    this->release();
    this->data = other.data;
    this->type_ = other.type_;
    this->is_ref = other.is_ref;
    this->ref_tar_ = other.ref_tar_;
    other.leak();
  }
}
void value::swap(value &other) {
  std::swap(this->data, other.data);
  std::swap(this->is_ref, other.is_ref);
  std::swap(this->ref_tar_, other.ref_tar_);
  std::swap(this->type_, other.type_);
}
value value::ref() const {
  value ret;
  ret.data = this->data;
  ret.is_ref = true;
  ret.type_ = this->type_;
  ret.ref_tar_ = const_cast<value *>(this);
  return ret;
}

void value::add(value const &v) {
  ERROR_EXIT_IF(v.type_ != this->type_, "Type Error, types do not match");
  switch (type_) {
  case types::OBJECT:
    ERROR_EXIT_IF(v.type_ != this->type_, "Type Error");
    break;
  case types::LIST:

    ERROR_EXIT_IF(v.type_ != this->type_, "Type Error");
    break;

  case types::STRING:
    this->as<string>() += v.as<string>();
    break;
  case types::FUNCTION:
    ERROR_EXIT_IF(v.type_ != this->type_, "Type Error");
    break;
  case types::NUMBER:
    this->as<number>() += v.as<number>();
    break;
  case types::UNDEFINED:
  default:
    break;
  }
}
void value::minus(value const &v) {
  ERROR_EXIT_IF(v.type_ != this->type_, "Type Error, types do not match");
  switch (type_) {
  case types::OBJECT:
  case types::LIST:
  case types::STRING:
  case types::FUNCTION:
    ERROR_EXIT_IF(v.type_ != this->type_, "Type Error");
    break;
  case types::NUMBER:
    this->as<number>() -= v.as<number>();
    break;
  case types::UNDEFINED:
  default:
    break;
  }
}
void value::multiply(value const &v) {
  ERROR_EXIT_IF(v.type_ != this->type_, "Type Error, types do not match");
  switch (type_) {
  case types::OBJECT:
  case types::LIST:
  case types::STRING:
  case types::FUNCTION:
    ERROR_EXIT_IF(v.type_ != this->type_, "Type Error");
    break;
  case types::NUMBER:
    this->as<number>() *= v.as<number>();
    break;
  case types::UNDEFINED:
  default:
    break;
  }
}
void value::divide(value const &v) {
  ERROR_EXIT_IF(v.type_ != this->type_, "Type Error, types do not match");
  switch (type_) {
  case types::OBJECT:
  case types::LIST:
  case types::STRING:
  case types::FUNCTION:
    ERROR_EXIT_IF(v.type_ != this->type_, "Type Error");
    break;
  case types::NUMBER:
    this->as<number>() /= v.as<number>();
    break;
  case types::UNDEFINED:
  default:
    break;
  }
}
void value::mold(value const &v) {
  ERROR_EXIT_IF(v.type_ != this->type_, "Type Error, types do not match");
  switch (type_) {
  case types::OBJECT:
  case types::LIST:
  case types::STRING:
  case types::FUNCTION:
    ERROR_EXIT_IF(v.type_ != this->type_, "Type Error");
    break;
  case types::NUMBER:
    this->as<number>() =
        (long long)(this->as<number>()) % (long long)(v.as<number>());
    break;
  case types::UNDEFINED:
  default:
    break;
  }
}

value value::copy() const { return static_cast<const value &>(*this); }

bool value::operator==(const value &other) const {
  if (other.type_ != this->type_) {
    return false;
  }

  switch (type_) {
  case types::OBJECT:
    return as<object>() == other.as<object>();
  case types::LIST:
    return as<list>() == other.as<list>();
  case types::STRING:
    return as<string>() == other.as<string>();
  case types::FUNCTION:
    return as<function>() == other.as<function>();
  case types::NUMBER: {
    return as<number>() == other.as<number>();
  }
  case types::UNDEFINED:
  default:
    break;
  }
  return false;
}

bool value::is_null() const { return this->type_ == types::UNDEFINED; }

std::string value::to_string(int level) const {
  switch (type_) {
  case types::OBJECT: {

    auto &obj_ref = as<object>();
    auto obj_size = obj_ref.size();
    std::string ret = "<object: size = " + std::to_string(obj_size) + ">\n{\n";
    size_t count = 0;
    for (auto &i : obj_ref) {
      if (count == obj_size - 1) {
        ret += "\"" + i.first + "\":" + i.second.to_string(level + 1);
      } else {
        ret += "\"" + i.first + "\":" + i.second.to_string(level + 1) + ",\n";
      }
      ++count;
    }
    ret += "\n}";
    return ret;
  }

  case types::LIST:
    return "<list: size = " + std::to_string(as<list>().size()) + ">";
  case types::STRING:
    return as<string>();
  case types::FUNCTION:
    return "<function>";
  case types::UNDEFINED:
    return "<null>";
  case types::NUMBER:
    return std::to_string(as<number>());
  default:
    return "<unknown>";
  }
}

FLOW_NAMESPACE_END

#endif
