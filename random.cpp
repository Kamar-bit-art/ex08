#include "random.hpp"

uint64_t Random::next() {
  state *= 6364136223846793005ul;
  state += 1442695040888963407ul;
  assert(state);
  return state;
}

// generates a random integer
uint32_t Random::generate() {
  next();
  return state >> 32;
}
int Random::generate_int() { return (int)generate(); }
bool Random::generate_bool() { return generate() < 2147483648u; }

// Generate an integer value in the range '[l,r]'.
int Random::pick_int(int l, int r) {
  assert(l <= r);
  const unsigned delta = 1 + r - (unsigned)l;
  unsigned tmp = generate(), scaled;
  if (delta) {
    const double fraction = tmp / 4294967296.0;
    scaled = delta * fraction;
  } else
    scaled = tmp;
  const int res = scaled + l;
  assert(l <= res);
  assert(res <= r);
  return res;
}