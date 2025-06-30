#include "logic_builder.hpp"
#include "logic_node.hpp"
#include <memory>
#include <vector>
#include <cassert>
#include <iostream>

int main() {
  Logic_Builder builder;
  
  // Test 1: Basic simplification and structural sharing
  std::cout << "Test 1: Basic simplification and structural sharing" << std::endl;
  std::vector<std::shared_ptr<Logic_Node>> args1;
  args1.push_back(builder.make_variable(1));
  args1.push_back(builder.make_variable(2));
  args1.push_back(builder.make_true());

  std::shared_ptr<Logic_Node> g1 = builder.make_conjunction(args1);
  std::cout << "Original formula: " << *g1 << std::endl;
  
  auto h1 = builder.simplify(g1);
  std::cout << "Simplified formula: " << *h1 << std::endl;
  
  // Should have 2 children after simplification (True is removed)
  assert(h1->arity() == 2);
  
  // Test 2: Verify caching works
  std::cout << "\nTest 2: Verify caching works" << std::endl;
  auto h2 = builder.simplify(g1);
  // Both simplifications should return the same object (pointer)
  std::cout << "Simplified formula (second time): " << *h2 << std::endl;
  std::cout << "Are pointers identical? " << (h1 == h2 ? "Yes" : "No") << std::endl;
  assert(h1 == h2); // Should be the same pointer (perfect sharing)
  
  // Test 3: Create structurally identical but different formulas
  std::cout << "\nTest 3: Create structurally identical but different formulas" << std::endl;
  std::vector<std::shared_ptr<Logic_Node>> args2;
  args2.push_back(builder.make_variable(1));
  args2.push_back(builder.make_variable(2));
  
  std::shared_ptr<Logic_Node> g2 = builder.make_conjunction(args2);
  std::cout << "Second formula: " << *g2 << std::endl;
  
  auto h3 = builder.simplify(g2);
  std::cout << "Simplified second formula: " << *h3 << std::endl;
  
  // Both formulas should be structurally equal after simplification
  std::cout << "Are simplified formulas structurally equal? " << (*h1 == *h3 ? "Yes" : "No") << std::endl;
  assert(*h1 == *h3); // Should be structurally equal
  
  // Test 4: AND[x] = x simplification
  std::cout << "\nTest 4: AND[x] = x simplification" << std::endl;
  std::vector<std::shared_ptr<Logic_Node>> args3;
  args3.push_back(builder.make_variable(5));
  
  std::shared_ptr<Logic_Node> g3 = builder.make_conjunction(args3);
  std::cout << "Single child formula: " << *g3 << std::endl;
  
  auto h4 = builder.simplify(g3);
  std::cout << "Simplified single child formula: " << *h4 << std::endl;
  
  // Should be simplified to just the variable
  auto var = std::dynamic_pointer_cast<Variable>(h4);
  assert(var != nullptr);
  assert(var->getLiteral() == 5);
  
  // Test 5: OR[x] = x simplification
  std::cout << "\nTest 5: OR[x] = x simplification" << std::endl;
  std::vector<std::shared_ptr<Logic_Node>> args4;
  args4.push_back(builder.make_variable(7));
  
  std::shared_ptr<Logic_Node> g4 = builder.make_disjunction(args4);
  std::cout << "Single child OR formula: " << *g4 << std::endl;
  
  auto h5 = builder.simplify(g4);
  std::cout << "Simplified single child OR formula: " << *h5 << std::endl;
  
  // Should be simplified to just the variable
  var = std::dynamic_pointer_cast<Variable>(h5);
  assert(var != nullptr);
  assert(var->getLiteral() == 7);
  
  // Test 6: AND[] = True simplification
  std::cout << "\nTest 6: AND[] = True simplification" << std::endl;
  std::vector<std::shared_ptr<Logic_Node>> args5;
  
  std::shared_ptr<Logic_Node> g5 = builder.make_conjunction(args5);
  std::cout << "Empty AND formula: " << *g5 << std::endl;
  
  auto h6 = builder.simplify(g5);
  std::cout << "Simplified empty AND formula: " << *h6 << std::endl;
  
  // Should be simplified to True
  auto constant = std::dynamic_pointer_cast<Constant>(h6);
  assert(constant != nullptr);
  assert(constant->getValue() == true);
  
  // Test 7: OR[] = False simplification
  std::cout << "\nTest 7: OR[] = False simplification" << std::endl;
  std::vector<std::shared_ptr<Logic_Node>> args6;
  
  std::shared_ptr<Logic_Node> g6 = builder.make_disjunction(args6);
  std::cout << "Empty OR formula: " << *g6 << std::endl;
  
  auto h7 = builder.simplify(g6);
  std::cout << "Simplified empty OR formula: " << *h7 << std::endl;
  
  // Should be simplified to False
  constant = std::dynamic_pointer_cast<Constant>(h7);
  assert(constant != nullptr);
  assert(constant->getValue() == false);
  
  std::cout << "\nAll tests passed!" << std::endl;
  return 0;
}