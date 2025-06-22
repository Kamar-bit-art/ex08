#include "logic_node.hpp"
#include "logic_builder.hpp"

// TODO exercise 0, 1, 2, and 5
// Stream operator implementation
std::ostream &operator<<(std::ostream &stream, const Logic_Node &n) {
    // Basic implementation for now
    if (dynamic_cast<const Constant*>(&n)) {
        stream << (dynamic_cast<const Constant*>(&n)->getValue() ? "True" : "False");
    } else if (dynamic_cast<const Variable*>(&n)) {
        stream << "x" << dynamic_cast<const Variable*>(&n)->getLiteral();
    } else if (const Gate* gate = dynamic_cast<const Gate*>(&n)) {
        stream << (gate->getType() == Gate_Type::AND_GATE ? "AND[" : "OR[");
        const auto& children = gate->getChildren();
        for (size_t i = 0; i < children.size(); ++i) {
            if (i > 0) stream << ", ";
            stream << *children[i];
        }
        stream << "]";
    } else {
        stream << "Unknown";
    }
    return stream;
}

// Constant implementation
Constant::Constant(bool value) : value(value) {}

size_t Constant::arity() const {
    return 0;
}

bool Constant::evaluation(const std::vector<bool> &/* inputs */) const {
    return value;
}

bool Constant::operator==(const Logic_Node *const other) const {
    if (const Constant* c = dynamic_cast<const Constant*>(other)) {
        return value == c->value;
    }
    return false;
}

bool Constant::operator==(const Logic_Node &other) const {
    return operator==(&other);
}

bool Constant::getValue() const {
    return value;
}

// Gate implementation
Gate::Gate(Gate_Type type, std::vector<std::shared_ptr<Logic_Node>> inputs)
    : kind(type), children(std::move(inputs)) {}

size_t Gate::arity() const {
    return children.size();
}

bool Gate::evaluation(const std::vector<bool> &inputs) const {
    // Handle special cases: AND[] = True, OR[] = False
    if (children.empty()) {
        return kind == Gate_Type::AND_GATE;
    }
    
    // Evaluate based on gate type
    if (kind == Gate_Type::AND_GATE) {
        // AND gate: all inputs must be true
        for (const auto& child : children) {
            if (!child->evaluation(inputs)) {
                return false;
            }
        }
        return true;
    } else { // OR_GATE
        // OR gate: at least one input must be true
        for (const auto& child : children) {
            if (child->evaluation(inputs)) {
                return true;
            }
        }
        return false;
    }
}

bool Gate::operator==(const Logic_Node *const other) const {
    if (const Gate* g = dynamic_cast<const Gate*>(other)) {
        if (kind != g->kind || children.size() != g->children.size()) {
            return false;
        }
        
        // Check if all children match (order matters)
        for (size_t i = 0; i < children.size(); ++i) {
            if (!(*children[i] == *g->children[i])) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Gate::operator==(const Logic_Node &other) const {
    return operator==(&other);
}

Gate_Type Gate::getType() const {
    return kind;
}

const std::vector<std::shared_ptr<Logic_Node>>& Gate::getChildren() const {
    return children;
}

std::vector<std::shared_ptr<Logic_Node>>& Gate::getChildrenMutable() {
    return children;
}

// Variable implementation
Variable::Variable(int literal) : literal(literal) {}

size_t Variable::arity() const {
    return 1;
}

bool Variable::evaluation(const std::vector<bool> &inputs) const {
    // Check if the literal is within range
    int index = abs(literal) - 1;
    if (index >= 0 && index < static_cast<int>(inputs.size())) {
        return literal > 0 ? inputs[index] : !inputs[index];
    }
    // Default behavior for out-of-range literals
    return false;
}

bool Variable::operator==(const Logic_Node *const other) const {
    if (const Variable* v = dynamic_cast<const Variable*>(other)) {
        return literal == v->literal;
    }
    return false;
}

bool Variable::operator==(const Logic_Node &other) const {
    return operator==(&other);
}

int Variable::getLiteral() const {
    return literal;
}