#include "logic_builder.hpp"
#include "logger.hpp"
#include "logic_node.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <unordered_set>

bool Logic_Node_Equal::operator()(
    const std::shared_ptr<Logic_Node> &lhs,
    const std::shared_ptr<Logic_Node> &rhs) const {
  if (lhs == rhs) return true; // Same pointer
  if (!lhs || !rhs) return false; // One is null
  return *lhs == *rhs; // Use the overloaded operator== from Logic_Node
}

size_t
Logic_Node_Hash::operator()(const std::shared_ptr<Logic_Node> &lhs) const {
  // Simple hash function for now, will be improved in exercise 5
  if (!lhs) return 0;
  
  // Use pointer value as hash for now
  return std::hash<const Logic_Node*>()(lhs.get());
}

Logic_Builder::simplifier_cache Logic_Builder::simplified_representative;

std::shared_ptr<Logic_Node> Logic_Builder::make_variable(int literal) {
  return std::make_shared<Variable>(literal);
}

std::shared_ptr<Logic_Node> Logic_Builder::make_conjunction(
    std::vector<std::shared_ptr<Logic_Node>> children) {
  // Handle special cases
  if (children.empty()) {
    return make_true(); // AND[] = True
  }
  
  // Check for False constants - if any child is False, the result is False
  for (const auto& child : children) {
    if (auto constant = std::dynamic_pointer_cast<Constant>(child)) {
      if (!constant->getValue()) { // False found
        return make_false();
      }
    }
  }
  
  // Filter out True constants as they don't affect the result
  std::vector<std::shared_ptr<Logic_Node>> filtered_children;
  for (const auto& child : children) {
    if (auto constant = std::dynamic_pointer_cast<Constant>(child)) {
      if (constant->getValue()) { // True constant
        continue; // Skip True constants
      }
    }
    filtered_children.push_back(child);
  }
  
  // If all children were filtered out (all were True), return True
  if (filtered_children.empty()) {
    return make_true();
  }
  
  // If only one child remains, return it directly
  if (filtered_children.size() == 1) {
    return filtered_children[0];
  }
  
  // Create the conjunction gate with filtered children
  return std::make_shared<Gate>(Gate_Type::AND_GATE, std::move(filtered_children));
}

std::shared_ptr<Logic_Node> Logic_Builder::make_disjunction(
    std::vector<std::shared_ptr<Logic_Node>> children) {
  // Handle special cases
  if (children.empty()) {
    return make_false(); // OR[] = False
  }
  
  // Check for True constants - if any child is True, the result is True
  for (const auto& child : children) {
    if (auto constant = std::dynamic_pointer_cast<Constant>(child)) {
      if (constant->getValue()) { // True found
        return make_true();
      }
    }
  }
  
  // Filter out False constants as they don't affect the result
  std::vector<std::shared_ptr<Logic_Node>> filtered_children;
  for (const auto& child : children) {
    if (auto constant = std::dynamic_pointer_cast<Constant>(child)) {
      if (!constant->getValue()) { // False constant
        continue; // Skip False constants
      }
    }
    filtered_children.push_back(child);
  }
  
  // If all children were filtered out (all were False), return False
  if (filtered_children.empty()) {
    return make_false();
  }
  
  // If only one child remains, return it directly
  if (filtered_children.size() == 1) {
    return filtered_children[0];
  }
  
  // Create the disjunction gate with filtered children
  return std::make_shared<Gate>(Gate_Type::OR_GATE, std::move(filtered_children));
}

std::shared_ptr<Logic_Node> Logic_Builder::make_true() {
  return std::make_shared<Constant>(true);
}

std::shared_ptr<Logic_Node> Logic_Builder::make_false() {
  return std::make_shared<Constant>(false);
}

void Logic_Builder::normalize(std::shared_ptr<Formula> f) {
  // Skip normalization for constants and variables
  if (std::dynamic_pointer_cast<Constant>(f) || std::dynamic_pointer_cast<Variable>(f)) {
    return;
  }
  
  // Cast to Gate to access children
  auto gate = std::dynamic_pointer_cast<Gate>(f);
  if (!gate) {
    return; // Not a gate, nothing to normalize
  }
  
  // First, normalize all children recursively
  auto& children = gate->getChildrenMutable();
  for (auto& child : children) {
    normalize(child);
  }
  
  // Remove duplicates
  std::unordered_set<std::shared_ptr<Logic_Node>, Logic_Node_Hash, Logic_Node_Equal> unique_children;
  std::vector<std::shared_ptr<Logic_Node>> filtered_children;
  
  for (const auto& child : children) {
    if (unique_children.insert(child).second) { // If insertion was successful (not a duplicate)
      filtered_children.push_back(child);
    }
  }
  
  // Update children with filtered list (no duplicates)
  children = std::move(filtered_children);
  
  // Handle constants based on gate type
  if (gate->getType() == Gate_Type::AND_GATE) {
    // For AND gate: if any child is False, replace with False
    // Remove True constants as they don't affect the result
    filtered_children.clear();
    bool has_false = false;
    
    for (const auto& child : children) {
      if (auto constant = std::dynamic_pointer_cast<Constant>(child)) {
        if (!constant->getValue()) { // False constant
          has_false = true;
          break;
        }
        // Skip True constants
        continue;
      }
      filtered_children.push_back(child);
    }
    
    if (has_false) {
      // Replace all children with a single False
      children.clear();
      children.push_back(make_false());
    } else {
      // Update children without True constants
      children = std::move(filtered_children);
    }
  } else if (gate->getType() == Gate_Type::OR_GATE) {
    // For OR gate: if any child is True, replace with True
    // Remove False constants as they don't affect the result
    filtered_children.clear();
    bool has_true = false;
    
    for (const auto& child : children) {
      if (auto constant = std::dynamic_pointer_cast<Constant>(child)) {
        if (constant->getValue()) { // True constant
          has_true = true;
          break;
        }
        // Skip False constants
        continue;
      }
      filtered_children.push_back(child);
    }
    
    if (has_true) {
      // Replace all children with a single True
      children.clear();
      children.push_back(make_true());
    } else {
      // Update children without False constants
      children = std::move(filtered_children);
    }
  }
  
  // Handle special cases
  if (children.empty()) {
    if (gate->getType() == Gate_Type::AND_GATE) {
      children.push_back(make_true()); // AND[] = True
    } else { // OR_GATE
      children.push_back(make_false()); // OR[] = False
    }
  }
}

std::shared_ptr<Formula> Logic_Builder::simplify(std::shared_ptr<Formula> f) {
  // Basic implementation for now, will be expanded in exercise 5
  normalize(f);
  return f;
}

bool Logic_Builder::evaluate(std::shared_ptr<Formula> f,
                             const std::vector<bool> &model) const {
  return f->evaluation(model);
}
