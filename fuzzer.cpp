#include "fuzzer.hpp"
#include "logic_builder.hpp"
#include "logic_node.hpp"
#include "random.hpp"

#include <iostream>
#include <memory>

// abort and print the current seed
void Fuzzer::abort_err() {
  std::cerr << "\nERROR, rerun with the following seed as start point "
            << current_loop_seed << "\n";
  ++found_errors;
  error_in_last_round = true;
  if (fail_on_first_error)
    abort();
}

void Fuzzer::print_banner(const std::string &str, int line) {
  std::cerr << "\n***" << str << "on line " << line << "**\n";
}

std::vector<std::shared_ptr<Formula>> Fuzzer::pick_children() {
  const int number_of_children = rand.pick_int(0, 6);
  std::vector<std::shared_ptr<Formula>> children;
  for (int i = 0; i < number_of_children; ++i) {
    const int pos = rand.pick_int(0, cache.size() - 1);
    auto child = cache[pos];
    children.push_back(child);
  }
  return children;
}

void Fuzzer::generate_model(std::vector<bool> &model) {
  model.clear ();
  for (int i = 0; i <= number_of_literals; ++i) {
    const bool positive = rand.generate_bool();
    model.push_back(positive);
  }
}

void Fuzzer::test_same_models (std::shared_ptr<Formula> f1, std::shared_ptr<Formula> f2) {
  const int n = rand.pick_int(0, 10000);
  std::vector<bool> model;
  for (int i = 0; i < n; ++i) {
    generate_model (model);
    const bool v1 = builder.evaluate(f1, model);
    const bool v2 = builder.evaluate(f2, model);
    if (v1 != v2) {
      std::cerr << "the models are not the same (val: " << v1 << ")\n\t" << *f1
                 << "\nvs (val: " << v2 << ")\n\t" << *f2 << "\n";
      std::cerr << "model: ";
      for (size_t i = 1; i < model.size (); ++i)
	std::cerr << (model[i] ? i : -i) << " ";
      std::cerr << "\n";
      abort_err();
      break;
    }
    model.clear ();
  }
}

void Fuzzer::produce_new_node (bool verbose) {
  std::string kind;

  const int n = rand.pick_int(0, 5);
  std::shared_ptr <Formula> formula;

  switch (n) {
  case 0:
    kind = "true";
    cache.push_back (builder.make_true ());
    break;
  case 1:
    kind = "false";
    cache.push_back (builder.make_false ());
    break;
  case 2:
    kind = "and";
    cache.push_back (builder.make_conjunction (pick_children()));
    break;
  case 3:
    kind = "or";
    cache.push_back (builder.make_disjunction (pick_children()));
    break;
  case 4:
  case 5:
    kind = "literal";
    cache.push_back (builder.make_variable (rand.pick_int(1, number_of_literals)));
    break;
  }
  if (verbose)
    std::cout << "produce new node " << kind << "\n";
}

void Fuzzer::test_normalize ([[maybe_unused]] bool verbose) {
  if (verbose)
    std::cout << "test normalize\n";
  const int pos = rand.pick_int(0, cache.size() - 1);
  std::shared_ptr<Logic_Node> orig = std::make_shared<Constant>(true); // setting a default value
  orig = cache[pos];
  std::shared_ptr<Logic_Node> child = std::make_shared<Constant>(true); // setting a default value
  child = orig;
  builder.normalize(child);

  test_same_models(child, orig);
  cache.push_back(child);
}


void Fuzzer::test_simplify ([[maybe_unused]] bool verbose) {
  const int pos = rand.pick_int(0, cache.size() - 1);
  auto orig = cache [pos];
  if (verbose)
    std::cout << "*****************\ntest simplify" << "\n";
  auto simplified = builder.simplify(orig);
  if (verbose)
    std::cout << "test simplify\t" << *orig << "\nafter simplification\t"
              << *simplified << "\n";

  // First, test that the simplified formula is semantically equivalent to the original
  test_same_models(simplified, orig);
  
  // Only perform structural checks on gates, not on constants or variables
  auto gate = std::dynamic_pointer_cast<Gate>(simplified);
  if (gate) {
    const auto& direct_children = gate->getChildren();
    
    // Check for constants in direct children of AND/OR gates
    // Constants should have been simplified away according to the rules
    for (const auto& child : direct_children) {
      if (std::dynamic_pointer_cast<Constant>(child)) {
        if (verbose)
          std::cout << "Error: Found constant in direct children of simplified formula" << std::endl;
        abort_err();
      }
    }
    
    // Check for duplicated nodes (perfect structural sharing)
    for (size_t i = 0; i < direct_children.size(); ++i) {
      for (size_t j = i + 1; j < direct_children.size(); ++j) {
        if (*direct_children[i] == *direct_children[j]) {
          if (verbose)
            std::cout << "Error: Found duplicate nodes, imperfect structural sharing" << std::endl;
          abort_err();
        }
      }
    }
  }
  
  // Always add the simplified formula to the cache
  cache.push_back(simplified);
}

void Fuzzer::prepopulate () {
  for (int i = 0; i < 10; ++i) {

  const int n = rand.pick_int(0, 5);

  switch (n) {
  case 0:
    cache.push_back (builder.make_true ());
    break;
  case 1:
    cache.push_back (builder.make_false ());
    break;
  case 2:
    [[fallthrough]]; // to increase likelyhood of creating variables
  case 3:
    [[fallthrough]];
  case 4:
    [[fallthrough]];
  case 5:
    cache.push_back (builder.make_variable (rand.pick_int(1, number_of_literals) * (rand.generate_bool() ? 1 : -1)));
    break;
  }
  }
}

void Fuzzer::run([[maybe_unused]] bool verbose) {

  // first populate the cache with some value
  prepopulate();

  // now test
  for (int i = 0; i < length; ++i) {
    if (!(i % 100))
      std::cout << "..." << i;

    const int n = rand.pick_int(0, 3);

    switch (n) {
    case 0:
      produce_new_node(verbose);
      break;
    case 1:
      test_normalize(verbose);
      break;
    case 2:
      test_simplify (verbose);
      break;
    default:
      if (rand.pick_int(0,100) < 10) {
	if (verbose)
	  std::cout << "emptying cache";
	cache.clear ();
	current_loop_seed = rand.seed();
	prepopulate();
	builder.clear_cache();
      }
      break;
    }

    if (error_in_last_round) {
      error_in_last_round = false;
    }
  }
  if (!fail_on_first_error) {
    std::cout << "\n\nerrors: " << found_errors << " from " << length << "\n";
  }
}
