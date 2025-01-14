#ifndef __FLOW_RUNTIME_ERROR_H__
#define __FLOW_RUNTIME_ERROR_H__

#include "./context.hpp"

FLOW_NAMESPACE_BEGIN

struct error {

  enum class code : int {
    UNKNOWN_ERROR = 0xff,
  };

  code ec = code::UNKNOWN_ERROR;

  std::string msg() const {
    switch (ec) {
    case code::UNKNOWN_ERROR:
      return "Unknown Error";

    default:
      return "Unknown Error";
    }
  }

  static void rise(error const &err) {
    printf("[ERROR] %s", err.msg().c_str());
  }
};

FLOW_NAMESPACE_END
#endif
