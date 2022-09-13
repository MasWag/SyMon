#pragma once

#include <variant>
#include <vector>
#include <array>
#include <string>

#include "common_types.hh"

template<typename T>
void insert_sorted(std::vector<T> &vec, const T &value) {
  auto it = std::lower_bound(vec.begin(), vec.end(), value);
  if (it == vec.end()) {
    vec.push_back(value);
  } else if (*it != value) {
    vec.insert(it, value);
  }
}

namespace Symbolic {
  // Symbolic valuation over strings
  using StringValuation = std::vector<std::variant<std::vector<std::string>, std::string>>;

  /*!
   * @brief Merge two string valuations if possible
   *
   * The merging is based on the following rewriting, where C is the intersection of A and B.
   * (x != A[0] && x != A[1] && ... && x != A[N-1]) || (x != B[0] && x != B[1] && ... && x != B[M-1]) = (x != C[0] && x != C[1] && ... && x != C[O-1])
   * @pre left.size() == right.size()
   */
  static inline std::optional<StringValuation> merge(const StringValuation &left, const StringValuation &right) {
    assert(left.size() == right.size());
    StringValuation result;
    result.reserve(left.size());
    for (int i = 0; i < left.size(); ++i) {
      if (left.at(i).index() == 1 || right.at(i).index() == 1) {
        return std::nullopt;
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

    return result;
  }

  static inline void pairwise_reduce(std::vector<StringValuation> &value) {
    std::vector<StringValuation> result;
    for (const auto &elem : value) {
      bool merged = false;
      for (auto &resultElem : result) {
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
    @brief Constraint on strings

    @note Currently, we assume that we know the value of at least one of the children.
  */
  struct StringConstraint {
    std::array<StringAtom, 2> children;
    enum class kind_t {
      EQ, NE
    } kind;

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
