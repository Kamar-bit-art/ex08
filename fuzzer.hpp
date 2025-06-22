#ifndef FUZZER_HPP
#define FUZZER_HPP

// TODO: if did not follow the template, this might not be the right file name
#include "logic_builder.hpp"

#include "random.hpp"

#include <list>
#include <stdint.h>
#include <string>

struct Clause;

class Fuzzer {
public:
  // Constructor with 3 arguments: the seed, the number of literals to use and
  // the number of tests to run.
  Fuzzer(uint64_t seed, int size, int len)
      : rand(seed), number_of_literals(size), length(len) {}

  void run(bool verbose);

protected:
  void produce_new_node(bool);
  std::vector<std::shared_ptr<Formula>> pick_children();
  void test_normalize(bool);
  void test_simplify(bool);
  void test_same_models(std::shared_ptr<Formula>, std::shared_ptr<Formula>);
  void generate_model(std::vector<bool> &model);
  void prepopulate();

private:
  // deterministic random generator
  Random rand;

  Logic_Builder builder;

  // highest literal to produce in formulas. This is the maximum and is
  // *reached*. It is not a size.
  int number_of_literals = 100;

  // number of tests to execute
  int length = 0;

  // seed of the current loop to be able to restart the search directly at that
  // point
  uint64_t current_loop_seed;

  // prints the same string to make it easier to identify which test failed.
  // Used at each test beginning with the second argument __LINE__ to give
  // precise feedback.
  void print_banner(const std::string &, int);
  uint64_t found_errors = 0;
  bool fail_on_first_error = true;
  bool error_in_last_round = false;

  // abort and prints the current seed
  void abort_err();

  // cache to reuse trees
  std::vector<std::shared_ptr<Formula>> cache;
};

#endif
