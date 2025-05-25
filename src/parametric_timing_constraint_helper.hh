/*
 * @author Masaki Waga
 * @date 2019-01-28
 */

#ifndef DATAMONITOR_PARAMETRIC_TIMING_CONSTRAINT_HELPER_HH
#define DATAMONITOR_PARAMETRIC_TIMING_CONSTRAINT_HELPER_HH

#include <iostream>

#include "parametric_timing_constraint.hh"

/*
 * @brief Parse Parametric Timing Constraint
 */
struct ParametricTimingConstraintHelper {
public:
  enum class kind_t {
    VARIABLE, PARAMETER, CONSTANT
  };
  enum class op_t {
    PLUS, MINUS
  };
  enum class comparison_t {
    LT, LE, EQ, GE, GT
  };
  using atom_t = std::pair<std::variant<std::size_t, Parma_Polyhedra_Library::Coefficient>, kind_t>;
  //! @brief head of each expression
  std::array<atom_t, 2> head;
  //! @brief tail of each expression
  std::array<std::vector<std::pair<op_t, atom_t>>, 2> tail;
  //! @brief comparison in the constraint
  comparison_t comparison = comparison_t::EQ;

  void
  toExpr(const std::size_t parameterSize, const atom_t &atom, Parma_Polyhedra_Library::Linear_Expression &expr) const {
    switch (atom.second) {
      case kind_t::VARIABLE:
        expr = Parma_Polyhedra_Library::Variable(parameterSize + std::get<std::size_t>(atom.first));
        break;
      case kind_t::PARAMETER:
        expr = Parma_Polyhedra_Library::Variable(std::get<std::size_t>(atom.first));
        break;
      case kind_t::CONSTANT:
        expr = Parma_Polyhedra_Library::Linear_Expression{std::get<Parma_Polyhedra_Library::Coefficient>(atom.first)};
        break;
    }
  }

  void extract(const std::size_t parameterSize, Parma_Polyhedra_Library::Constraint &constraint) const {
    std::array<Parma_Polyhedra_Library::Linear_Expression, 2> expr;
    for (std::size_t i = 0; i < 2; i++) {
      toExpr(parameterSize, head[i], expr[i]);
      for (const auto &elem: tail[i]) {
        Parma_Polyhedra_Library::Linear_Expression tmpExpr;
        toExpr(parameterSize, elem.second, tmpExpr);
        switch (elem.first) {
          case op_t::PLUS:
            expr[i] += tmpExpr;
            break;
          case op_t::MINUS:
            expr[i] -= tmpExpr;
            break;
        }
      }
    }
    switch (comparison) {
      case comparison_t::LT:
        constraint = expr[0] < expr[1];
        break;
      case comparison_t::LE:
        constraint = expr[0] <= expr[1];
        break;
      case comparison_t::EQ:
        constraint = expr[0] == expr[1];
        break;
      case comparison_t::GE:
        constraint = expr[0] >= expr[1];
        break;
      case comparison_t::GT:
        constraint = expr[0] > expr[1];
        break;
    }
  }
};


static std::istream &skipBlank(std::istream &is) {
  while (true) {
    switch (is.peek()) {
      case ' ':
      case '\t':
      case '\n':
        is.get(); // Consume a whitespace
        continue;
      default:
        return is;
    }
  }
}

static std::istream &readAtom(std::istream &is, ParametricTimingConstraintHelper::atom_t &atom) {
  skipBlank(is);
  // Here, we terminate if the stream is not good, including EOF or fail state.
  if (!is.good()) {
    return is;
  }

  switch (is.peek()) {
    case 'x': {
      is.get(); // consume 'x'
      atom.second = ParametricTimingConstraintHelper::kind_t::VARIABLE;
      std::size_t id;
      is >> id;
      // Here and below, we check if the input stream is not in a fail state, excluding EOF.
      if (is.fail()) {
        return is;
      }
      atom.first = id;
      break;
    }
    case 'p': {
      is.get(); // consume 'p'
      atom.second = ParametricTimingConstraintHelper::kind_t::PARAMETER;
      std::size_t id;
      is >> id;
      if (is.fail()) {
        return is;
      }
      atom.first = id;
      break;
    }
    default : {
      atom.second = ParametricTimingConstraintHelper::kind_t::CONSTANT;
      Parma_Polyhedra_Library::Coefficient constant;
      is >> constant;
      if (is.fail()) {
        return is;
      }
      atom.first = constant;
    }
  }
  return is;
}

static inline
std::istream &operator>>(std::istream &is, ParametricTimingConstraintHelper &helper) {
  std::size_t position = 0;
  while (position < 2) {
    readAtom(is, helper.head[position]);
    if (!is.good()) {
      return is;
    }
    bool keepHere = true;
    while (keepHere) {
      skipBlank(is);
      switch (is.get()) {
        case '+': {
          std::pair<ParametricTimingConstraintHelper::op_t, ParametricTimingConstraintHelper::atom_t> elem;
          elem.first = ParametricTimingConstraintHelper::op_t::PLUS;
          readAtom(is, elem.second);
          helper.tail[position].emplace_back(std::move(elem));
          break;
        }
        case '-': {
          std::pair<ParametricTimingConstraintHelper::op_t, ParametricTimingConstraintHelper::atom_t> elem;
          elem.first = ParametricTimingConstraintHelper::op_t::MINUS;
          readAtom(is, elem.second);
          helper.tail[position].emplace_back(std::move(elem));
          break;
        }
        case '<': {
          keepHere = false;
          if (position++ != 0) {
            is.unget();
            is.setstate(std::ios::failbit);
            return is;
          }
          if (is.get() == '=') {
            helper.comparison = ParametricTimingConstraintHelper::comparison_t::LE;
          } else {
            is.unget();
            helper.comparison = ParametricTimingConstraintHelper::comparison_t::LT;
          }
          break;
        }
        case '>': {
          keepHere = false;
          if (position++ != 0) {
            is.unget();
            is.setstate(std::ios::failbit);
            return is;
          }
          if (is.get() == '=') {
            helper.comparison = ParametricTimingConstraintHelper::comparison_t::GE;
          } else {
            is.unget();
            helper.comparison = ParametricTimingConstraintHelper::comparison_t::GT;
          }
          break;
        }
        case '=': {
          keepHere = false;
          if (position++ != 0) {
            is.unget();
            is.setstate(std::ios::failbit);
            return is;
          }
          // we allow both == and = for eq
          if (is.get() == '=') {
            helper.comparison = ParametricTimingConstraintHelper::comparison_t::EQ;
          } else {
            is.unget();
            helper.comparison = ParametricTimingConstraintHelper::comparison_t::EQ;
          }
          break;
        }
        default:
          is.unget();
          if (position != 1) {
            is.setstate(std::ios::failbit);
          }
          return is;
      }
    }
  }
  return is;
}

static inline
std::ostream &operator<<(std::ostream &is, const ParametricTimingConstraintHelper::op_t &op) {
  switch (op) {
    case ParametricTimingConstraintHelper::op_t::PLUS:
      is << "plus";
      break;
    case ParametricTimingConstraintHelper::op_t::MINUS:
      is << "minus";
      break;
  }
  return is;
}

static inline
std::ostream &operator<<(std::ostream &is, const ParametricTimingConstraintHelper::kind_t &kind) {
  switch (kind) {
    case ParametricTimingConstraintHelper::kind_t::CONSTANT:
      is << "constant";
      break;
    case ParametricTimingConstraintHelper::kind_t::VARIABLE:
      is << "variable";
      break;
    case ParametricTimingConstraintHelper::kind_t::PARAMETER:
      is << "parameter";
      break;
  }
  return is;
}

static inline
std::ostream &operator<<(std::ostream &is, const ParametricTimingConstraintHelper::comparison_t &comparison) {
  switch (comparison) {
    case ParametricTimingConstraintHelper::comparison_t::LE:
      is << "<=";
      break;
    case ParametricTimingConstraintHelper::comparison_t::LT:
      is << "<";
      break;
    case ParametricTimingConstraintHelper::comparison_t::EQ:
      is << "==";
      break;
    case ParametricTimingConstraintHelper::comparison_t::GT:
      is << ">";
      break;
    case ParametricTimingConstraintHelper::comparison_t::GE:
      is << ">=";
      break;
  }
  return is;
}

static inline
std::ostream &operator<<(std::ostream &is, const ParametricTimingConstraintHelper &helper) {
  is << "stub\n";
  return is;
}

#endif //DATAMONITOR_PARAMETRIC_TIMING_CONSTRAINT_HELPER_HH
