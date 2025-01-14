#ifndef __FLOW_RUNTIME_STATE_H__
#define __FLOW_RUNTIME_STATE_H__

// the state machine of the language

#include "exception.hpp"

FLOW_NAMESPACE_BEGIN

enum class state : int {

  // reading spaces/newlines
  //
  // $ -> DEF_VAR_START
  // :{ -> DEF_FN_START
  // ; -> NONE
  // import -> IMPORT
  // @ -> FN_ARG_GET
  // \w+ -> VAR_ASSIGN_LEFT
  NONE = 0xff,

  // ; -> NONE
  EXPECTING_END,

  // define variables

  // \w+ -> DEF_VAR_END
  DEF_VAR_START,

  // = -> R_EXPRESSION
  // ; -> NONE
  DEF_VAR_END, // waiting for ';' or '='
  DEF_VAR_WAITING_R_VALUE,

  DEF_FN_START, // read f{

  // // \w+\s*; -> NONE
  // VAR_ASSIGN_LEFT,
  L_EXPRESSION,
  R_EXPRESSION,

  FN_CALL,

  FN_ARG_GET,

  IMPORT,
};

class state_machine {
private:
  std::stack<state> states;

  static void state_assert(bool b) {
    assert(b);
    // TODO: use struct error
  }

public:
  state_machine() : states() { states.push(state::NONE); }

  state current() const { return states.top(); }

  void
  when(std::unordered_map<state, std::function<void(void)>> const &states) {
    assert(states.count(current()));
    states.at(current())();
  }

  state previous() {
    state temp = current();
    states.pop();
    state ret = current();
    states.push(temp);
    return ret;
  }

  // cast to next state
  state cast(state next_state = state::NONE) {
    switch (current()) {
    case state::NONE:

      break;
    case state::DEF_VAR_START:
      state_assert(next_state == state::DEF_VAR_END);
      break;

    default:
      break;
    }

    states.pop();
    states.push(next_state);
    return current();
  }
};

FLOW_NAMESPACE_END

#endif
