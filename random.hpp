#ifndef _random_hpp_INCLUDED
#define _random_hpp_INCLUDED

#include <cassert>
#include <cstdint>

// Deterministic random number generator taken from CaDiCaL
//
// There is not much to see here, but in short it is class with a seed called
// `state`. This seed is changed whenever a new value is derived.  The random
// generator is deterministic if initialized with the same seed.
class Random {

  uint64_t state;
  // increase the current state of the computer
  void add(uint64_t a) {
    if (!(state += a))
      state = 1;
    next();
  }

public:
  // initialize the seed with a giveen seed
  Random(uint64_t seed) : state(seed) {}
  // updates the state to a given value
  void operator=(uint64_t seed) { state = seed; }

  // increases the internal state to initialize the seed to a better value
  void operator+=(uint64_t a) { add(a); }

  // internal seed
  uint64_t seed() const { return state; }

  // increases the internal state
  uint64_t next();

  // generates a random integer
  uint32_t generate();
  int generate_int();
  bool generate_bool();

  // Generate an integer value in the range '[l,r]'.
  int pick_int(int l, int r);
};

#endif
