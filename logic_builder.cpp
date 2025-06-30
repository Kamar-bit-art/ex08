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
  if (!lhs) return 0;
  
  // Hash based on the type and content of the node
  if (auto constant = std::dynamic_pointer_cast<Constant>(lhs)) {
    // Hash for constants: 1 for True, 0 for False
    return constant->getValue() ? 1 : 0;
  } 
  else if (auto variable = std::dynamic_pointer_cast<Variable>(lhs)) {
    // Hash for variables: use the literal value
    return std::hash<int>()(variable->getLiteral()) * 31;
  } 
  else if (auto gate = std::dynamic_pointer_cast<Gate>(lhs)) {
    // Hash for gates: combine gate type and children hashes
    size_t hash_value = (gate->getType() == Gate_Type::AND_GATE) ? 17 : 23;
    
    // Combine with children hashes
    for (const auto& child : gate->getChildren()) {
      hash_value = hash_value * 31 + (*this)(child);
    }
    
    return hash_value;
  }
  
  // Fallback to pointer hash if type is unknown
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

std::vector<std::shared_ptr<Formula>> Logic_Builder::collect_children(std::shared_ptr<Formula> f) {
  std::vector<std::shared_ptr<Formula>> result;
  
  // Base case: if f is null, return empty vector
  if (!f) {
    return result;
  }
  
  // Add the current node
  result.push_back(f);
  
  // If it's a gate, recursively collect children
  auto gate = std::dynamic_pointer_cast<Gate>(f);
  if (gate) {
    for (const auto& child : gate->getChildren()) {
      // Don't include the child itself again (it will be included in its own recursive call)
      auto child_nodes = collect_children(child);
      result.insert(result.end(), child_nodes.begin(), child_nodes.end());
    }
  }
  
  return result;
}

std::shared_ptr<Formula> Logic_Builder::simplify(std::shared_ptr<Formula> f) {
  // Check if we've already simplified this formula
  auto it = simplified_representative.find(f);
  if (it != simplified_representative.end()) {
    return it->second; // Return cached result
  }
  
  // Handle base cases: constants and variables don't need simplification
  if (std::dynamic_pointer_cast<Constant>(f) || std::dynamic_pointer_cast<Variable>(f)) {
    // Store in cache and return
    simplified_representative[f] = f;
    return f;
  }
  
  // Cast to Gate to access children
  auto gate = std::dynamic_pointer_cast<Gate>(f);
  if (!gate) {
    // Not a gate, just return as is
    return f;
  }
  
  // Get gate type and children
  Gate_Type type = gate->getType();
  std::vector<std::shared_ptr<Logic_Node>> children = gate->getChildren();
  
  // Recursively simplify all children first
  std::vector<std::shared_ptr<Logic_Node>> simplified_children;
  for (const auto& child : children) {
    simplified_children.push_back(simplify(child));
  }
  
  // Create a new gate with simplified children
  std::shared_ptr<Formula> result;
  
  // Apply simplification rules based on gate type
  if (type == Gate_Type::AND_GATE) {
    // AND[] = True
    if (simplified_children.empty()) {
      result = make_true();
      simplified_representative[f] = result;
      return result;
    }
    
    // AND[x] = x
    if (simplified_children.size() == 1) {
      result = simplified_children[0];
      simplified_representative[f] = result;
      return result;
    }
    
    // NEW RULE: AND[... False ...] = False
    for (const auto& child : simplified_children) {
      if (auto constant = std::dynamic_pointer_cast<Constant>(child)) {
        if (!constant->getValue()) { // Found False
          result = make_false();
          simplified_representative[f] = result;
          return result;
        }
      }
    }
    
    // Filter out True constants as they don't affect the result
    std::vector<std::shared_ptr<Logic_Node>> filtered_children;
    for (const auto& child : simplified_children) {
      if (auto constant = std::dynamic_pointer_cast<Constant>(child)) {
        if (constant->getValue()) { // True constant
          continue; // Skip True constants
        }
      }
      filtered_children.push_back(child);
    }
    
    // If all children were filtered out (all were True), return True
    if (filtered_children.empty()) {
      result = make_true();
      simplified_representative[f] = result;
      return result;
    }
    
    // If only one child remains after filtering, return it
    if (filtered_children.size() == 1) {
      result = filtered_children[0];
      simplified_representative[f] = result;
      return result;
    }
    
    // Remove duplicates to ensure perfect structural sharing
    std::unordered_set<std::shared_ptr<Logic_Node>, Logic_Node_Hash, Logic_Node_Equal> unique_children;
    std::vector<std::shared_ptr<Logic_Node>> unique_filtered_children;
    
    for (const auto& child : filtered_children) {
      if (unique_children.insert(child).second) { // If insertion was successful (not a duplicate)
        unique_filtered_children.push_back(child);
      }
    }
    
    result = std::make_shared<Gate>(Gate_Type::AND_GATE, std::move(unique_filtered_children));
  } 
  else { // OR_GATE
    // OR[] = False
    if (simplified_children.empty()) {
      result = make_false();
      simplified_representative[f] = result;
      return result;
    }
    
    // OR[x] = x
    if (simplified_children.size() == 1) {
      result = simplified_children[0];
      simplified_representative[f] = result;
      return result;
    }
    
    // NEW RULE: OR[... True ...] = True
    for (const auto& child : simplified_children) {
      if (auto constant = std::dynamic_pointer_cast<Constant>(child)) {
        if (constant->getValue()) { // Found True
          result = make_true();
          simplified_representative[f] = result;
          return result;
        }
      }
    }
    
    // Filter out False constants as they don't affect the result
    std::vector<std::shared_ptr<Logic_Node>> filtered_children;
    for (const auto& child : simplified_children) {
      if (auto constant = std::dynamic_pointer_cast<Constant>(child)) {
        if (!constant->getValue()) { // False constant
          continue; // Skip False constants
        }
      }
      filtered_children.push_back(child);
    }
    
    // If all children were filtered out (all were False), return False
    if (filtered_children.empty()) {
      result = make_false();
      simplified_representative[f] = result;
      return result;
    }
    
    // If only one child remains after filtering, return it
    if (filtered_children.size() == 1) {
      result = filtered_children[0];
      simplified_representative[f] = result;
      return result;
    }
    
    // Remove duplicates to ensure perfect structural sharing
    std::unordered_set<std::shared_ptr<Logic_Node>, Logic_Node_Hash, Logic_Node_Equal> unique_children;
    std::vector<std::shared_ptr<Logic_Node>> unique_filtered_children;
    
    for (const auto& child : filtered_children) {
      if (unique_children.insert(child).second) { // If insertion was successful (not a duplicate)
        unique_filtered_children.push_back(child);
      }
    }
    
    result = std::make_shared<Gate>(Gate_Type::OR_GATE, std::move(unique_filtered_children));
  }
  
  // Store the result in the cache
  simplified_representative[f] = result;
  
  return result;
}

bool Logic_Builder::evaluate(std::shared_ptr<Formula> f,
                             const std::vector<bool> &model) const {
  return f->evaluation(model);
}
