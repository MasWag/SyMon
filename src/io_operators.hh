#pragma once

#include <iostream>
#include <memory>

#include "non_symbolic_number_constraint.hh"
#include "non_symbolic_string_constraint.hh"
//#include "non_symbolic_update.hh"

#include "symbolic_number_constraint.hh"
#include "symbolic_string_constraint.hh"
#include "timing_constraint.hh"

namespace NonSymbolic {
  static inline std::ostream &operator<<(std::ostream &os, const NonSymbolic::StringAtom &atom) {
    switch (atom.value.index()) {
      case 0:
        os << "x" << std::get<0>(atom.value);
        break;
      case 1:
        os << std::get<1>(atom.value);
        break;
    }
    return os;
  }

  static inline std::istream &operator>>(std::istream &is, NonSymbolic::StringAtom &atom) {
    switch (is.get()) {
      case 'x': {
        VariableID id;
        is >> id;
        atom.value = id;
        break;
      }
      case '\'': {
        std::string str;
        if (std::getline(is, str, '\'')) {
          atom.value = std::move(str);
        } else {
          is.unget();
          is.setstate(std::ios_base::failbit);
        }
        break;
      }
      default:
        is.unget();
        is.setstate(std::ios_base::failbit);
    }
    return is;
  }

  static inline std::istream &operator>>(std::istream &is, NonSymbolic::StringConstraint::kind_t &kind) {
    std::string str;
    is >> str;
    if (str == "==") {
      kind = NonSymbolic::StringConstraint::kind_t::EQ;
    } else if (str == "!=") {
      kind = NonSymbolic::StringConstraint::kind_t::NE;
    } else {
      is.setstate(std::ios_base::failbit);
    }
    return is;
  }

  static inline std::istream &operator>>(std::istream &is, NonSymbolic::StringConstraint &constraint) {
    is >> constraint.children[0];
    if (is.get() != ' ') {
      is.setstate(std::ios_base::failbit);
      is.unget();
      return is;
    }
    is >> constraint.kind;
    if (is.get() != ' ') {
      is.setstate(std::ios_base::failbit);
      is.unget();
      return is;
    }
    is >> constraint.children[1];
    return is;
  }

  static inline std::ostream &operator<<(std::ostream &os, const NonSymbolic::StringConstraint::kind_t &kind) {
    switch (kind) {
      case NonSymbolic::StringConstraint::kind_t::EQ:
        os << " == ";
        break;
      case NonSymbolic::StringConstraint::kind_t::NE:
        os << " != ";
        break;
    }
    return os;
  }

  static inline std::ostream &operator<<(std::ostream &os, const NonSymbolic::StringConstraint &stringConstraint) {
    os << stringConstraint.children[0];
    os << stringConstraint.kind;
    os << stringConstraint.children[1];
    return os;
  }

  static inline std::ostream &operator<<(std::ostream &os,
                                         const std::vector<NonSymbolic::StringConstraint> &stringConstraints) {
    os << "{";
    for (const NonSymbolic::StringConstraint &stringConstraint: stringConstraints) {
      os << stringConstraint;
      os << ", ";
    }
    os << "}";
    return os;
  }

  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os, const NonSymbolic::NumberExpression<Number> &numberExpression) {
    switch (numberExpression.kind) {
      case NonSymbolic::NumberExpression<Number>::kind_t::ATOM:
        os << "x" << std::get<VariableID>(numberExpression.child);
        break;
      case NonSymbolic::NumberExpression<Number>::kind_t::CONSTANT:
        os << std::get<Number>(numberExpression.child);
        break;
      case NonSymbolic::NumberExpression<Number>::kind_t::PLUS:
        os << std::get<1>(numberExpression.child)[0];
        os << " + ";
        os << std::get<1>(numberExpression.child)[1];
        break;
      case NonSymbolic::NumberExpression<Number>::kind_t::MINUS:
        os << std::get<1>(numberExpression.child)[0];
        os << " - ";
        os << std::get<1>(numberExpression.child)[1];
        break;
    }
    return os;
  }

  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os, const typename NonSymbolic::NumberExpression<Number>::kind_t &kind) {
    switch (kind) {
      case NonSymbolic::NumberExpression<Number>::kind_t::ATOM:
        break;
      case NonSymbolic::NumberExpression<Number>::kind_t::CONSTANT:
        break;
      case NonSymbolic::NumberExpression<Number>::kind_t::PLUS:
        os << " + ";
        break;
      case NonSymbolic::NumberExpression<Number>::kind_t::MINUS:
        os << " - ";
        break;
    }
    return os;
  }

  // Concrete overload for the common Number = int case. Boost unit tests (and other code)
  // often stream values of type NonSymbolic::NumberExpression<int>::kind_t directly.
  // Template argument deduction doesn't work when the parameter is a nested dependent
  // type, so provide this non-template overload so streaming compiles cleanly.
  static inline std::ostream &operator<<(std::ostream &os, const NonSymbolic::NumberExpression<int>::kind_t &kind) {
    switch (kind) {
      case NonSymbolic::NumberExpression<int>::kind_t::ATOM:
        break;
      case NonSymbolic::NumberExpression<int>::kind_t::CONSTANT:
        break;
      case NonSymbolic::NumberExpression<int>::kind_t::PLUS:
        os << " + ";
        break;
      case NonSymbolic::NumberExpression<int>::kind_t::MINUS:
        os << " - ";
        break;
    }
    return os;
  }

  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os,
                                        const std::pair<VariableID, NonSymbolic::NumberExpression<Number>> &update) {
    os << "x" << update.first << " := " << update.second;
    return os;
  }

  template<typename Number>
  static inline std::istream &operator>>(std::istream &is, std::pair<VariableID, NonSymbolic::NumberExpression<Number>> &update) {
  if (is.get() != 'x') {
    is.unget();
    is.setstate(std::ios_base::failbit);
    return is;
  }
  is >> update.first;
  if (is.get() != ' ') {
    is.setstate(std::ios_base::failbit);
    is.unget();
    return is;
  }
  std::string str;
  is >> str;
  if (str != ":=") {
    is.setstate(std::ios_base::failbit);
  }
  if (is.get() != ' ') {
    is.unget();
    is.setstate(std::ios_base::failbit);
    return is;
  }
  is >> update.second;
  return is;
}

  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os,
                                        const std::vector<std::pair<VariableID, NonSymbolic::NumberExpression<Number>>> &updates) {
    os << "{";
    for (const auto &update: updates) {
      os << update;
      os << ", ";
    }
    os << "}";
    return os;
  }

  template <typename Number>
  static inline std::istream &operator>>(std::istream &is, NonSymbolic::NumberExpression<Number> &numberExpression) {
    //! @note I should rewrite it by lex/yacc if I want more expressiveness.
    if (is.peek() == 'x') {
      is.get();
      VariableID id;
      is >> id;
      numberExpression = NumberExpression<Number>(id);
    } else {
      int constant;
      is >> constant;
      numberExpression = NumberExpression<Number>::constant(constant);
    }
    while (is.good()) {
      if (is.get() != ' ') {
        is.unget();
        return is;
      }
      char op = is.get();
      if (op != '+' && op != '-') {
        is.unget();
        is.putback(' ');
        return is;
      }
      if (is.get() != ' ') {
        is.setstate(std::ios_base::failbit);
        is.unget();
        return is;
      }
      auto child = std::make_shared<NonSymbolic::NumberExpression<Number>>(numberExpression);
      std::shared_ptr<NonSymbolic::NumberExpression<Number>> leaf;
      if (is.peek() == 'x') {
        is.get();
        VariableID id;
        is >> id;
        leaf = std::make_shared<NonSymbolic::NumberExpression<Number>>(id);
      } else {
        int constant;
        is >> constant;
        leaf = std::make_shared<NonSymbolic::NumberExpression<Number>>(NumberExpression<Number>::constant(constant));
      }
      if (op == '+') {
        numberExpression = {NonSymbolic::NumberExpression<Number>::kind_t::PLUS, child, leaf};
      } else if (op == '-') {
        numberExpression = {NonSymbolic::NumberExpression<Number>::kind_t::MINUS, child, leaf};
      } else {
        is.setstate(std::ios_base::failbit);
        break;
      }
    }
    return is;
  }

  template <typename Number>
  static inline std::ostream &print(std::ostream &os,
                                    const typename NonSymbolic::NumberConstraint<Number>::kind_t kind) {
    switch (kind) {
      case NonSymbolic::NumberConstraint<Number>::kind_t::GT:
        os << " > ";
        break;
      case NonSymbolic::NumberConstraint<Number>::kind_t::GE:
        os << " >= ";
        break;
      case NonSymbolic::NumberConstraint<Number>::kind_t::EQ:
        os << " == ";
        break;
      case NonSymbolic::NumberConstraint<Number>::kind_t::NE:
        os << " != ";
        break;
      case NonSymbolic::NumberConstraint<Number>::kind_t::LE:
        os << " <= ";
        break;
      case NonSymbolic::NumberConstraint<Number>::kind_t::LT:
        os << " < ";
        break;
    }
    return os;
  }

  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os,
                                         const typename NonSymbolic::NumberConstraint<Number>::kind_t kind) {
    return print(os, kind);
  }

  template <typename Number>
  static inline std::ostream &operator<<(std::ostream &os,
                                         const NonSymbolic::NumberConstraint<Number> &numberConstraint) {
    os << numberConstraint.left;
    print<Number>(os, numberConstraint.kind);
    os << numberConstraint.right;
    return os;
  }

  template <typename Number>
  static inline std::istream &scan(std::istream &is, typename NonSymbolic::NumberConstraint<Number>::kind_t &kind) {
    std::string str;
    is >> str;
    if (str == ">") {
      kind = NonSymbolic::NumberConstraint<Number>::kind_t::GT;
    } else if (str == ">=") {
      kind = NonSymbolic::NumberConstraint<Number>::kind_t::GE;
    } else if (str == "==") {
      kind = NonSymbolic::NumberConstraint<Number>::kind_t::EQ;
    } else if (str == "!=") {
      kind = NonSymbolic::NumberConstraint<Number>::kind_t::NE;
    } else if (str == "<=") {
      kind = NonSymbolic::NumberConstraint<Number>::kind_t::LE;
    } else if (str == "<") {
      kind = NonSymbolic::NumberConstraint<Number>::kind_t::LT;
    } else {
      is.setstate(std::ios_base::failbit);
    }
    return is;
  }

  template <typename Number>
  static inline std::istream &operator>>(std::istream &is, NonSymbolic::NumberConstraint<Number> &numberConstraint) {
    is >> numberConstraint.left;
    if (is.get() != ' ') {
      is.setstate(std::ios_base::failbit);
      is.unget();
      return is;
    }
    scan<Number>(is, numberConstraint.kind);
    if (is.get() != ' ') {
      is.setstate(std::ios_base::failbit);
      is.unget();
      return is;
    }
    is >> numberConstraint.right;
    return is;
  }
} // namespace NonSymbolic

namespace Symbolic {
  static inline std::ostream &operator<<(std::ostream &os, const Symbolic::StringAtom &atom) {
    switch (atom.value.index()) {
      case 0:
        os << "x" << std::get<0>(atom.value);
        break;
      case 1:
        os << std::get<1>(atom.value);
        break;
    }
    return os;
  }

  static inline std::istream &operator>>(std::istream &is, Symbolic::StringAtom &atom) {
    switch (is.get()) {
      case 'x': {
        VariableID id;
        is >> id;
        atom.value = id;
        break;
      }
      case '\'': {
        std::string str;
        if (std::getline(is, str, '\'')) {
          atom.value = std::move(str);
        } else {
          is.unget();
          is.setstate(std::ios_base::failbit);
        }
        break;
      }
      default:
        is.unget();
        is.setstate(std::ios_base::failbit);
    }
    return is;
  }

  static inline std::istream &operator>>(std::istream &is, Symbolic::StringConstraint::kind_t &kind) {
    std::string str;
    is >> str;
    if (str == "==") {
      kind = Symbolic::StringConstraint::kind_t::EQ;
    } else if (str == "!=") {
      kind = Symbolic::StringConstraint::kind_t::NE;
    } else {
      is.setstate(std::ios_base::failbit);
    }
    return is;
  }

  static inline std::istream &operator>>(std::istream &is, Symbolic::StringConstraint &constraint) {
    is >> constraint.children[0];
    if (is.get() != ' ') {
      is.setstate(std::ios_base::failbit);
      is.unget();
      return is;
    }
    is >> constraint.kind;
    if (is.get() != ' ') {
      is.setstate(std::ios_base::failbit);
      is.unget();
      return is;
    }
    is >> constraint.children[1];
    return is;
  }

  static inline std::ostream &operator<<(std::ostream &os, const Symbolic::StringConstraint::kind_t &kind) {
    switch (kind) {
      case Symbolic::StringConstraint::kind_t::EQ:
        os << " == ";
        break;
      case Symbolic::StringConstraint::kind_t::NE:
        os << " != ";
        break;
    }
    return os;
  }

  static inline std::ostream &operator<<(std::ostream &os, const Symbolic::StringConstraint &stringConstraint) {
    os << stringConstraint.children[0];
    os << stringConstraint.kind;
    os << stringConstraint.children[1];
    return os;
  }

  static inline std::ostream &operator<<(std::ostream &os,
                                         const std::vector<Symbolic::StringConstraint> &stringConstraints) {
    os << "{";
    for (const Symbolic::StringConstraint &stringConstraint: stringConstraints) {
      os << stringConstraint;
      os << ", ";
    }
    os << "}";
    return os;
  }
} // namespace Symbolic

static inline std::ostream &operator<<(std::ostream &os, const std::pair<VariableID, VariableID> &update) {
  os << "x" << update.first << " := x" << update.second;
  return os;
}

static inline std::istream &operator>>(std::istream &is, std::pair<VariableID, VariableID> &update) {
  if (is.get() != 'x') {
    is.unget();
    is.setstate(std::ios_base::failbit);
    return is;
  }
  is >> update.first;
  if (is.get() != ' ') {
    is.setstate(std::ios_base::failbit);
    is.unget();
    return is;
  }
  std::string str;
  is >> str;
  if (str != ":=") {
    is.setstate(std::ios_base::failbit);
  }
  if (is.get() != ' ') {
    is.unget();
    is.setstate(std::ios_base::failbit);
    return is;
  }
  if (is.get() != 'x') {
    is.unget();
    is.setstate(std::ios_base::failbit);
    return is;
  }
  is >> update.second;
  return is;
}

static inline std::ostream &operator<<(std::ostream &os,
                                       const std::vector<std::pair<VariableID, VariableID>> &updates) {
  os << "{";
  for (const auto &update: updates) {
    os << update;
    os << ", ";
  }
  os << "}";
  return os;
}

template <class Number>
static inline std::ostream &operator<<(std::ostream &os,
                                       const std::vector<NonSymbolic::NumberConstraint<Number>> &vector) {
  os << "{";
  for (const auto &element: vector) {
    os << element;
    os << " , ";
  }
  os << "}";
  return os;
}

static inline std::istream &operator>>(std::istream &is, Symbolic::NumberExpression &numberExpression) {
  //! @note I should rewrite it by lex/yacc if I want more expressiveness.
  if (is.peek() == 'x') {
    is.get();
    VariableID id;
    is >> id;
    numberExpression = Parma_Polyhedra_Library::Variable(id);
  } else {
    int constant;
    is >> constant;
    numberExpression = Symbolic::NumberExpression{constant};
  }
  while (is.good()) {
    if (is.get() != ' ') {
      is.unget();
      return is;
    }
    char op = is.get();
    if (op != '+' && op != '-') {
      is.unget();
      is.putback(' ');
      return is;
    }
    if (is.get() != ' ') {
      is.setstate(std::ios_base::failbit);
      is.unget();
      return is;
    }
    Symbolic::NumberExpression leaf;
    if (is.peek() == 'x') {
      is.get();
      VariableID id;
      is >> id;
      leaf = Parma_Polyhedra_Library::Variable(id);
    } else {
      int constant;
      is >> constant;
      leaf = Symbolic::NumberExpression{constant};
    }
    if (op == '+') {
      numberExpression += leaf;
    } else if (op == '-') {
      numberExpression -= leaf;
    } else {
      is.setstate(std::ios_base::failbit);
      break;
    }
  }
  return is;
}

static inline std::istream &operator>>(std::istream &is, Symbolic::NumberConstraint &numberConstraint) {
  std::array<Symbolic::NumberExpression, 2> expr;
  is >> expr[0];
  if (is.get() != ' ') {
    is.setstate(std::ios_base::failbit);
    is.unget();
    return is;
  }
  std::string str;
  is >> str;
  if (is.get() != ' ') {
    is.setstate(std::ios_base::failbit);
    is.unget();
    return is;
  }
  is >> expr[1];
  if (str == ">") {
    numberConstraint = expr[0] > expr[1];
  } else if (str == ">=") {
    numberConstraint = expr[0] >= expr[1];
  } else if (str == "==") {
    numberConstraint = expr[0] == expr[1];
  } else if (str == "<=") {
    numberConstraint = expr[0] <= expr[1];
  } else if (str == "<") {
    numberConstraint = expr[0] < expr[1];
  } else {
    is.setstate(std::ios_base::failbit);
  }
  return is;
}

static inline std::ostream &operator<<(std::ostream &os,
                                       const std::pair<VariableID, Symbolic::NumberExpression> &update) {
  using Parma_Polyhedra_Library::IO_Operators::operator<<;
  os << "x" << update.first << " := " << update.second;
  return os;
}

static inline std::istream &operator>>(std::istream &is, std::pair<VariableID, Symbolic::NumberExpression> &update) {
  if (is.get() != 'x') {
    is.unget();
    is.setstate(std::ios_base::failbit);
    return is;
  }
  is >> update.first;
  if (is.get() != ' ') {
    is.setstate(std::ios_base::failbit);
    is.unget();
    return is;
  }
  std::string str;
  is >> str;
  if (str != ":=") {
    is.setstate(std::ios_base::failbit);
  }
  if (is.get() != ' ') {
    is.unget();
    is.setstate(std::ios_base::failbit);
    return is;
  }
  is >> update.second;
  return is;
}

static inline std::ostream &operator<<(std::ostream &os,
                                       const std::vector<std::pair<VariableID, Symbolic::NumberExpression>> &updates) {
  os << "{";
  for (const auto &update: updates) {
    os << update;
    os << ", ";
  }
  os << "}";
  return os;
}

template <class T> static inline std::istream &operator>>(std::istream &is, std::vector<T> &resetVars) {
  resetVars.clear();
  if (!is) {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  if (is.get() != '{') {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  if (!is) {
    is.setstate(std::ios_base::failbit);
    return is;
  }

  while (true) {
    T x;
    is >> x;
    resetVars.emplace_back(std::move(x));
    if (!is) {
      is.setstate(std::ios_base::failbit);
      return is;
    }
    char c;
    is >> c;
    if (c == '}') {
      break;
    } else if (c == ',') {
      is >> c;
      if (c != ' ') {
        is.setstate(std::ios_base::failbit);
        return is;
      }
    } else {
      is.setstate(std::ios_base::failbit);
      return is;
    }
  }

  return is;
}

static inline std::ostream &operator<<(std::ostream &os, const std::vector<Symbolic::NumberConstraint> &vector) {
  using Parma_Polyhedra_Library::IO_Operators::operator<<;
  os << "{";
  for (const auto &element: vector) {
    os << element;
    os << " , ";
  }
  os << "}";
  return os;
}
