#ifndef __FLOW_DEFS_HPP__
#define __FLOW_DEFS_HPP__

#define FLOW_NAMESPACE_BEGIN namespace flow {
#define FLOW_NAMESPACE_END }

#define ERROR_EXIT(msg)                                                        \
  printf("ERROR: %s [%s:%i]\n", msg, __FILE__, __LINE__);                      \
  exit(-1)

#define ERROR_EXIT_IF(cond, msg)                                               \
  if (cond) {                                                                  \
    ERROR_EXIT(msg);                                                           \
  }                                                                            \
  void(0)

#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <set>
#include <stack>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#endif
