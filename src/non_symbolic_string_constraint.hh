#pragma once

#include <vector>
#include <array>
#include <string>
#include <variant>
#include <optional>

#include "common_types.hh"

namespace NonSymbolic {
  using StringValuation = std::vector<std::optional<std::string>>;

  struct StringAtom {
    std::variant<VariableID, std::string> value;

    void eval(const StringValuation &env, std::variant<VariableID, std::string> &result) const {
      result = value;
      if (result.index() == 1) {
        return;
      } else if (env.at(std::get<std::size_t>(result))) {
        result = *env.at(std::get<std::size_t>(result));
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
            env[std::get<0>(evaluated[0])] = disabledString;
            return true;
          } else if (evaluated[0].index() == 1 && evaluated[1].index() == 0) {
            const std::string &disabledString = std::get<std::string>(evaluated[0]);
            env[std::get<0>(evaluated[1])] = disabledString;
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
      if (!env[assignedID]) {
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
    explicit SCMaker(VariableID id) : first({id}) {}

    StringConstraint operator==(const std::string &str) const {
      const StringAtom second{str};
      return {{first, second}, StringConstraint::kind_t::EQ};
    }

    StringConstraint operator==(const VariableID id) const {
      const StringAtom second{id};
      return {{first, second}, StringConstraint::kind_t::EQ};
    }

    StringConstraint operator!=(const std::string &str) const {
      const StringAtom second{str};
      return {{first, second}, StringConstraint::kind_t::NE};
    }

    StringConstraint operator!=(const VariableID id) const {
      const StringAtom second{id};
      return {{first, second}, StringConstraint::kind_t::NE};
    }

  private:
    const StringAtom first;
  };
}
