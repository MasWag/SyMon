#pragma once

#include <array>
#include <string>
#include <variant>
#include <vector>

#include "common_types.hh"

template <typename T> void insert_sorted(std::vector<T> &vec, const T &value) {
  auto it = std::lower_bound(vec.begin(), vec.end(), value);
  if (it == vec.end()) {
    vec.push_back(value);
  } else if (*it != value) {
    vec.insert(it, value);
  }
}

namespace Symbolic {
  using StringValuation = std::vector<std::variant<std::vector<std::string>, std::string>>;

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
    enum class kind_t { EQ, NE } kind;

    bool eval(StringValuation &env) const {
      std::array<std::variant<VariableID, std::string>, 2> evaluated;
      for (int i = 0; i < 2; i++) {
        children[i].eval(env, evaluated[i]);
      }
      switch (kind) {
        case kind_t::EQ: {
          if (evaluated[0].index() == 0 && evaluated[1].index() == 0) {
            throw "Unimplemented case: At least one of the children must have a concrete value. "
                  "StringConstraint";
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
            throw "Unimplemented case: At least one of the children must have a concrete value. "
                  "StringConstraint";
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
    explicit SCMaker(VariableID id) : first({id}) {
    }

    StringConstraint operator==(const std::string &str) {
      const StringAtom second{str};
      return {{first, second}, StringConstraint::kind_t::EQ};
    }

    StringConstraint operator==(const VariableID id) {
      const StringAtom second{id};
      return {{first, second}, StringConstraint::kind_t::EQ};
    }

    StringConstraint operator!=(const std::string &str) {
      const StringAtom second{str};
      return {{first, second}, StringConstraint::kind_t::NE};
    }

    StringConstraint operator!=(const VariableID id) {
      const StringAtom second{id};
      return {{first, second}, StringConstraint::kind_t::NE};
    }

  private:
    const StringAtom first;
  };
} // namespace Symbolic
