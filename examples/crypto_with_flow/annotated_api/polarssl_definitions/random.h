#ifndef RANDOM_H
#define RANDOM_H

#include "macro_defines.h"

/*@

predicate random_request(int principal, int info, bool key) = true;

predicate random_state_predicate(predicate(void *) state_pred) = true;

#define PRG_PRECONDITION(STATE_PRED, STATE) \
  [?f]STATE_PRED(STATE) &*& \
  random_request(?principal, ?info, ?key_request) &*& \
  generated_values(principal, ?count) &*& \
  chars(output, len, _) &*& len >= MIN_RANDOM_SIZE
  
#define PRG_POSTCONDITION(STATE_PRED, STATE) \
  [f]STATE_PRED(STATE) &*& \
  generated_values(principal, count + 1) &*& \
  result == 0 ? \
    cryptogram(output, len, ?cs, ?cg) &*& \
    info == cg_info(cg) &*& \
    key_request ? \
      cg == cg_symmetric_key(principal, count + 1) \
    : \
      cg == cg_random(principal, count + 1) \
  : \
    chars(output, len, _)

@*/

typedef int random_function/*@(predicate(void *) state_pred)@*/
                              (void* state, char* output, size_t len);
  //@ requires PRG_PRECONDITION(state_pred, state);
  //@ ensures PRG_POSTCONDITION(state_pred, state);

#endif
