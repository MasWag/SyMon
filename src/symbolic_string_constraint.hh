#pragma once

#include <variant>
#include <vector>
#include <array>
#include <string>

#include "common_types.hh"
#include <iostream>

/*!
 * @file symbolic_string_constraint.hh
 * @author Masaki Waga
 * @brief Implementation of the symbolic constraints over strings.
 */

template<typename T>
void insert_sorted(std::vector<T> &vec, const T &value) {
  auto it = std::lower_bound(vec.begin(), vec.end(), value);
  if (it == vec.end()) {
    vec.push_back(value);
  } else if (*it != value) {
    vec.insert(it, value);
  }
}

namespace std {
  static inline std::ostream &operator<<(std::ostream &stream, const std::vector<std::string> &valuation) {
    for (const auto &str: valuation) {
      stream << "Var != " << str << ", ";
    }

    return stream;
  }
}
namespace Symbolic {
  /*!
   * @brief Symbolic valuation over strings
   *
   * A symbolic string valuation \f$v\f$ over variables \f$x_1,x_2, \dots, x_n\f$ is such that \f$\bigwedge_{i \in \{1,2,\dots,n\}} v_i\f$, where
   * \f$v_i\f$ is either \f$x_i = s\f$ or \f$x_i \not\in \{s_1,s_2,\dots,s_m\}\f$ for strings \f$s, s_1, s_2, \dots s_m\f$.
   */
  using StringValuation = std::vector<std::variant<std::vector<std::string>, std::string>>;

  static inline std::ostream &operator<<(std::ostream &stream, const StringValuation &valuation) {
    stream << "{";
    for (int i = 0; i < valuation.size(); ++i) {
      if (valuation.at(i).index() == 0) {
        for (const auto &str: std::get<std::vector<std::string>>(valuation.at(i))) {
          stream << "x" << i << " != " << str << ", ";
        }
      } else {
        stream << "x" << i << " == " << std::get<std::string>(valuation.at(i)) << ", ";
      }
    }
    stream << "}";

    return stream;
  }

  /*!
   * @brief Merge two symbolic string valuations if possible
   *
   * @retval result Returns the merged symbolic string valuation if the merging succeeded.
   * @retval std::nullopt When the merging failed.
   *
   * The merging is based on the following observation.
   * Let \f$v\f$ and \f$v'\f$ be symbolic string valuations such that \f$x \not\in S\f$ and \f$x \not\in S'\}\f$, where
   * \f$S = \{s_1,s_2,\dots,s_m\}\f$ and \f$S' = \{s'_1,s'_2,\dots,s'_{m'}\f$.
   * The disjunction \f$v \lor v'\f$ is \f$x \not\in S \lor x \not\in S'\f$, which is \f$x \not\in S \cap S'\f$.
   * Also, for symbolic string valuations \f$x = s\f$ and \f$x \not\in S\}\f$, where \f$s \in S\f$,
   * their disjunction is \f$x \not\in S \setminus \{s\}\f$.
   *
   * @pre left.size() == right.size()
   * @post left.size() == result.size() if the merging succeeded.
   */
  static inline std::optional<StringValuation> merge(const StringValuation &left, const StringValuation &right) {
    assert(left.size() == right.size());
    StringValuation result;
    result.reserve(left.size());
    for (int i = 0; i < left.size(); ++i) {
      if (left.at(i) == right.at(i)) {
        result.push_back(left.at(i));
      } else if (left.at(i).index() == 1 && right.at(i).index() == 1) {
        return std::nullopt;
      } else if (left.at(i).index() == 1) {
        const std::string value = std::get<std::string>(left.at(i));
        auto rightVector = std::get<std::vector<std::string>>(right.at(i));
        auto it = std::find(rightVector.begin(), rightVector.end(), value);
        if (it == rightVector.end()) {
          return std::nullopt;
        } else {
          rightVector.erase(it);
          result.push_back(rightVector);
        }
      } else if (right.at(i).index() == 1) {
        const std::string value = std::get<std::string>(right.at(i));
        auto leftVector = std::get<std::vector<std::string>>(left.at(i));
        auto it = std::find(leftVector.begin(), leftVector.end(), value);
        if (it == leftVector.end()) {
          return std::nullopt;
        } else {
          leftVector.erase(it);
          result.push_back(leftVector);
        }
      } else {
        auto leftElem = left.at(i);
        std::sort(std::get<0>(leftElem).begin(), std::get<0>(leftElem).end());
        auto rightElem = right.at(i);
        std::sort(std::get<0>(rightElem).begin(), std::get<0>(rightElem).end());
        std::vector<std::string> resultElem;
        resultElem.reserve(std::get<0>(leftElem).size());
        std::set_intersection(std::get<0>(leftElem).begin(), std::get<0>(leftElem).end(),
                              std::get<0>(rightElem).begin(), std::get<0>(rightElem).end(),
                              std::back_inserter(resultElem));
        result.push_back(resultElem);
      }
    }

    assert(left.size() == result.size());
    return result;
  }

  static inline void pairwise_reduce(std::vector<StringValuation> &value) {
    std::vector<StringValuation> result;
    for (const auto &elem: value) {
      bool merged = false;
      for (auto &resultElem: result) {
        const auto opt = merge(elem, resultElem);
        if (opt) {
          resultElem = *opt;
          merged = true;
          break;
        }
      }
      if (!merged) {
        result.push_back(elem);
      }
    }

    value = result;
  }

  struct StringAtom {
    std::variant<VariableID, std::string> value;

    void eval(const StringValuation &env, std::variant<VariableID, std::string> &result) const {
      result = value;
      if (result.index() == 1) {
        return;
      }
      if (env.at(std::get<std::size_t>(result)).index() == 1) {
        result = std::get<std::string>(env.at(std::get<std::size_t>(result)));
      }
    }
  };

  /*!
   * @brief Constraint on strings
   *
   * @note Currently, we assume that we know the value of at least one of the children.
   */
  struct StringConstraint {
    std::array<StringAtom, 2> children;
    enum class kind_t {
      EQ, NE
    } kind;

    /*!
     * @brief Constrain the given symbolic string valuation with this constraint
     *
     * @param[in,out] env The symbolic string valuation to be constrained
     * @retval true If the conjunction of the given symbolic string valuation and this constraint is satisfiable.
     * @retval false If the conjunction of the given symbolic string valuation and this constraint is unsatisfiable.
     */
    bool eval(StringValuation &env) const {
      std::array<std::variant<VariableID, std::string>, 2> evaluated;
      for (int i = 0; i < 2; i++) {
        children[i].eval(env, evaluated[i]);
      }
      switch (kind) {
        case kind_t::EQ: {
          if (evaluated[0].index() == 0 && evaluated[1].index() == 0) {
            throw "Unimplemented case: At least one of the children must have a concrete value. StringConstraint";
          } else if (evaluated[0].index() == 0 && evaluated[1].index() == 1) {
            const VariableID assignedID = std::get<VariableID>(evaluated[0]);
            const std::string &assignedString = std::get<std::string>(evaluated[1]);
            return assignIfPossible(env, assignedID, assignedString);
          } else if (evaluated[0].index() == 1 && evaluated[1].index() == 0) {
            const VariableID assignedID = std::get<VariableID>(evaluated[1]);
            const std::string &assignedString = std::get<std::string>(evaluated[0]);
            return assignIfPossible(env, assignedID, assignedString);
          } else {
            return std::get<std::string>(evaluated[0]) == std::get<std::string>(evaluated[1]);
          }
          break;
        }
        case kind_t::NE: {
          if (evaluated[0].index() == 0 && evaluated[1].index() == 0) {
            throw "Unimplemented case: At least one of the children must have a concrete value. StringConstraint";
          } else if (evaluated[0].index() == 0 && evaluated[1].index() == 1) {
            const std::string &disabledString = std::get<std::string>(evaluated[1]);
            insert_sorted(std::get<0>(env[std::get<0>(evaluated[0])]), disabledString);
            return true;
          } else if (evaluated[0].index() == 1 && evaluated[1].index() == 0) {
            const std::string &disabledString = std::get<std::string>(evaluated[0]);
            insert_sorted(std::get<0>(env[std::get<0>(evaluated[1])]), disabledString);
            return true;
          } else {
            return std::get<std::string>(evaluated[0]) != std::get<std::string>(evaluated[1]);
          }
        }
      }
      return false;
    }

  private:
    bool assignIfPossible(StringValuation &env, const VariableID assignedID, const std::string &assignedString) const {
      std::vector<std::string> &disabledStrings = std::get<0>(env[assignedID]);
      if (!std::binary_search(disabledStrings.begin(), disabledStrings.end(), assignedString)) {
        // assignedString is not disabled
        env[assignedID] = assignedString;
        return true;
      } else {
        // assignedString is not disabled
        return false;
      }
    }
  };

  /*!
   * @brief Helper class to construct Symbolic::StrincConstraint
   *
   * For example, the string constraint \f$x_0 \neq \text{"foo"}\f$ can be constructed as follows.
   * @code
   * SCMaker(0) != "foo"
   * @endcode
   */
  class SCMaker {
  public:
    SCMaker(VariableID id) : first({id}) {
    }

    StringConstraint operator==(std::string str) {
      StringAtom second{str};
      return {{first, second}, StringConstraint::kind_t::EQ};
    }

    StringConstraint operator!=(std::string str) {
      StringAtom second{str};
      return {{first, second}, StringConstraint::kind_t::NE};
    }

  private:
    const StringAtom first;
  };
}
