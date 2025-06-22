#ifndef LOGIC_HPP
#define LOGIC_HPP

#include <memory>
#include <unordered_map>
#include <vector>

class Logic_Node;
class Gate;
class Variable;
class Constant;
class Logger;

typedef Logic_Node Formula;

struct Logic_Node_Equal {
  bool operator()(const std::shared_ptr<Logic_Node> &lhs,
                  const std::shared_ptr<Logic_Node> &rhs) const;
};
struct Logic_Node_Hash {
  size_t operator()(const std::shared_ptr<Logic_Node> &rhs) const;
};

// Function declarations
class Logic_Builder {
private:
public:
  std::shared_ptr<Formula> make_variable(int literal);
  std::shared_ptr<Formula>
  make_conjunction(std::vector<std::shared_ptr<Formula>> children);
  std::shared_ptr<Formula>
  make_disjunction(std::vector<std::shared_ptr<Formula>> children);
  std::shared_ptr<Formula> make_true();
  std::shared_ptr<Formula> make_false();

  std::shared_ptr<Formula> simplify (std::shared_ptr<Formula>);

  void normalize (std::shared_ptr<Formula>f);
  bool evaluate (std::shared_ptr<Formula> f, const std::vector<bool> &model) const;
  using simplifier_cache = std::unordered_map<std::shared_ptr<Formula>, std::shared_ptr<Formula> /* Do you need this?, Logic_Node_Hash, Logic_Node_Equal*/>; // TODO exercise 5

  void clear_cache() { simplified_representative.clear(); } // for the fuzzer

protected:
  static simplifier_cache simplified_representative;
};

#endif // LOGIC_HPP
