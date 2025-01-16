#ifndef __FLOW_TYPE_TRAITS_HPP__
#define __FLOW_TYPE_TRAITS_HPP__

#include "defs.hpp"

FLOW_NAMESPACE_BEGIN

namespace traits {

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

} // namespace traits

FLOW_NAMESPACE_END

#endif
