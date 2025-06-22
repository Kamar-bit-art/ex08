#include "logic_builder.hpp"
#include "logic_node.hpp"
#include <memory>
#include <vector>
#include <cassert>


int main () {
  // simple example, feel free to change anything you want
  std::vector<std::shared_ptr<Logic_Node>> args;
  Logic_Builder builder;
  args.push_back(builder.make_variable(1));
  args.push_back(builder.make_variable(2));
  args.push_back(builder.make_true ());

  std::shared_ptr<Logic_Node>g = builder.make_conjunction(args);

  // Update model to ensure it satisfies the formula (x1 AND x2 AND True)
  // For this formula to be true, both x1 and x2 must be true
  std::vector<bool> model {true, true, true, true};

  assert (!(g == args[0]));

  std::cout << *g << std::endl;
  for (bool b : model) std::cout << b << ' ';
  std::cout << std::endl;
  std::cout << "Evaluation result: " << g->evaluation(model) << std::endl;

  auto h = builder.simplify(g);
  assert (h->arity() == 2);
}