#ifndef LOGIC_NODE_HPP
#define LOGIC_NODE_HPP

#include <cassert>
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>

class Logic_Builder;

// Abstract base class for logic nodes (Formula)
class Logic_Node {
public:
  virtual ~Logic_Node() = default;
  virtual size_t arity() const = 0;

  virtual bool evaluation(const std::vector<bool> &inputs) const = 0;

  // Equality operators
  virtual bool operator==(const Logic_Node *const other) const = 0;
  virtual bool operator==(const Logic_Node &other) const = 0;

  friend std::ostream &operator<<(std::ostream &stream, const Logic_Node &n);
  friend class Logic_Builder;
};

// Class representing a constant (True/False)
class Constant : public Logic_Node {
public:
  Constant(bool value);

  size_t arity() const override;
  bool evaluation(const std::vector<bool> &inputs) const override;
  bool operator==(const Logic_Node *const other) const override;
  bool operator==(const Logic_Node &other) const override;
  
  bool getValue() const;

private:
  bool value;
};

enum class Gate_Type { AND_GATE, OR_GATE };

// Class representing a gate (AND/OR)
class Gate : public Logic_Node {
public:
  Gate(Gate_Type type, std::vector<std::shared_ptr<Logic_Node>> inputs);
  virtual ~Gate() = default;
  
  size_t arity() const override;
  bool evaluation(const std::vector<bool> &inputs) const override;
  bool operator==(const Logic_Node *const other) const override;
  bool operator==(const Logic_Node &other) const override;

  Gate_Type getType() const;
  const std::vector<std::shared_ptr<Logic_Node>>& getChildren() const;
  std::vector<std::shared_ptr<Logic_Node>>& getChildrenMutable();

private:
  Gate_Type kind;
  std::vector<std::shared_ptr<Logic_Node>> children;
};

// Class representing a variable (literal)
class Variable : public Logic_Node {
public:
  Variable(int literal);

  size_t arity() const override;
  bool evaluation(const std::vector<bool> &inputs) const override;
  bool operator==(const Logic_Node *const other) const override;
  bool operator==(const Logic_Node &other) const override;
  
  int getLiteral() const;

private:
  int literal;
};

#endif // LOGIC_NODE_HPP
