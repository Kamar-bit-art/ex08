#include "fuzzer.hpp"
#include "random.hpp"

#include <iostream>
#include <string>
#include <time.h>

// Convert the RZ account to a proper seed
//
// We convert each char to a number and shift to get a proper number. Basically
// we see the number as a hex and ignore extra characters. Collisions do not
// really matter anyway.
uint64_t int_of_rz_account(std::string rz) {
  uint64_t seed = 0;
  for (const auto &c : rz) {
    seed *= 16;
    seed += c;
  }
  return seed;
}

// main function
//
// it support 4 different calls:
//
//   - 3 options with the starting seed, the number of tests, and any option
//   last to produce verbose output
//
//   - 2 options with only the starting RZ account and the number of tests. Then
//   verbose is not used.
//
//   - 1 options with the starting seed. Then it is running only 1000 tests.
//
//   - in all other cases, it just generates a seed based on the time
//
// Except for 2 options, we assume that the seed is a number.
int main(int argc, char **argv) {
  uint64_t seed;
  int length = 1001;
  bool verbose = false;
  if (argc == 4) { // verbose mode
    seed = static_cast<uint64_t>(std::stoull(argv[1]));
    length = static_cast<uint64_t>(std::stoul(argv[2])) + 1;
    std::cout << "using as seed " << seed << "\n";
    verbose = true;
  } else if (argc == 3) { // seed with length
    length = static_cast<uint64_t>(std::stoul(argv[2])) + 1;
    if (length == 2)
      seed = static_cast<uint64_t>(std::stoul(argv[1]));
    else
      seed = static_cast<uint64_t>(int_of_rz_account(argv[1]));
    std::cout << "using as seed " << seed << "\n";
  } else if (argc != 2) {
    std::cout << "generating new seed based on the time!\n";
    Random random_generater(time(NULL));
    seed = random_generater.generate_int();
    length = random_generater.pick_int(0, 100000);
  } else {
    std::cout << std::stoull(argv[1]) << "\n";
    seed = static_cast<uint64_t>(std::stoull(argv[1]));
    std::cout << "using as seed! " << seed << "\n";
  }
  std::cout << "testing " << length - 1 << " values\n";
  Fuzzer fuzz(seed, 20, length);
  fuzz.run(verbose);
  return 0;
}
