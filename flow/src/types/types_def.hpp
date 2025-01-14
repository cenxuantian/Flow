#ifndef __FLOW_TYPES_TYPES_DEF_HPP__
#define __FLOW_TYPES_TYPES_DEF_HPP__

// the state machine of the language

#include "../defs.hpp"

FLOW_NAMESPACE_BEGIN

enum class types : int {
  UNDEFINED,
  NUMBER,
  STRING,
  FUNCTION,
  OBJECT,
  LIST,
};

struct function;
class value;
using string = std::string;
using number = double;
using list = std::vector<value>;
using object = std::unordered_map<std::string, value>;

template <bool Cond, class Type> struct meta_case {
  static constexpr bool cond = Cond;
  using type = Type;
};

template <class Type> struct meta_case_default {
  static constexpr bool cond = true;
  using type = Type;
};

template <class Case, class... Cases> struct meta_select {
  using type = std::conditional_t<Case::cond, typename Case::type,
                                  typename meta_select<Cases...>::type>;
};

template <class Case> struct meta_select<Case> {
  using type = std::conditional_t<Case::cond, typename Case::type, void>;
};

template <class T> struct to_value_types {
  using in_type = std::decay_t<T>;
  using type = typename meta_select<
      meta_case<std::is_same_v<in_type, const char *>, string>,
      meta_case<std::is_same_v<in_type, char *>, string>,
      meta_case<std::is_same_v<in_type, string>, string>,
      meta_case<std::is_same_v<in_type, list>, list>,
      meta_case<std::is_same_v<in_type, object>, object>,
      meta_case<std::is_same_v<in_type, char>, number>,
      meta_case<std::is_same_v<in_type, unsigned char>, number>,
      meta_case<std::is_same_v<in_type, short>, number>,
      meta_case<std::is_same_v<in_type, unsigned short>, number>,
      meta_case<std::is_same_v<in_type, int>, number>,
      meta_case<std::is_same_v<in_type, unsigned int>, number>,
      meta_case<std::is_same_v<in_type, long long>, number>,
      meta_case<std::is_same_v<in_type, unsigned long long>, number>,
      meta_case<std::is_same_v<in_type, float>, number>,
      meta_case<std::is_same_v<in_type, double>, number>,
      meta_case<std::is_same_v<in_type, function>, function>,
      meta_case_default<void>>::type;
};

class value {
private:
  types type_;
  void *data;
  bool is_ref;
  value *ref_tar_;

  void release();
  void leak();

  template <class T> constexpr types get_type() const {
    if constexpr (std::is_same_v<T, number>) {
      return types::NUMBER;
    } else if constexpr (std::is_same_v<T, string>) {
      return types::STRING;
    } else if constexpr (std::is_same_v<T, function>) {
      return types::FUNCTION;
    } else if constexpr (std::is_same_v<T, list>) {
      return types::LIST;
    } else if constexpr (std::is_same_v<T, object>) {
      return types::OBJECT;
    } else {
      return types::UNDEFINED;
    }
  }

public:
  value();
  // construct from the input value
  // will move the v
  template <class T, typename std::enable_if_t<!std::is_same_v<
                         typename to_value_types<T>::type, void>> * = nullptr>
  value(T &&v)
      : type_(get_type<typename to_value_types<T>::type>()),
        data(new typename to_value_types<T>::type(v)), is_ref(false),
        ref_tar_(nullptr) {}
  // move constructor
  value(value &&other) noexcept;
  // copy constructor
  value(value const &other);
  ~value();

  value &operator=(value &&) = delete;
  value &operator=(value const &) = delete;

  void store(value &&other);
  void swap(value &other);
  types type() const;
  value ref() const;
  value copy() const;
  bool operator==(const value &other) const;
  template <class T> const T &as() const {
    assert(get_type<T>() == type_);
    return *reinterpret_cast<T *>(data);
  }
  template <class T> T &as() {
    assert(get_type<T>() == type_);
    return *reinterpret_cast<T *>(data);
  }

  bool is_null() const;
  std::string to_string(int level = 0) const;
  void add(value const &v);
  void minus(value const &v);
  void multiply(value const &v);
  void divide(value const &v);
  void mold(value const &v);
};

struct function {
public:
  enum class types {
    SCRIPT_FN,
    CPP_FN,
  };
  types type_;
  value out_;
  std::string inner_text;
  std::function<void(list const &)> cpp_fn;
  function();
  function(std::string const &);
  function(std::function<void(list const &)> const &);
  bool operator==(function const &other) const;
  value &&get_output() const;
};

FLOW_NAMESPACE_END

#endif
