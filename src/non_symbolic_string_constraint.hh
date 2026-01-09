#pragma once

#include <array>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "common_types.hh"

namespace NonSymbolic {
  using StringValuation = std::vector<std::optional<std::string>>;

  struct StringAtom {
    std::variant<VariableID, std::string> value;

    void eval(const StringValuation &env, std::variant<VariableID, std::string> &result) const {
      if (std::holds_alternative<std::string>(value)) {
        result = std::get<std::string>(value);
      } else if (env.at(std::get<std::size_t>(value))) {
        result = *env.at(std::get<std::size_t>(value));
      } else {
        result = std::get<VariableID>(value);
      }
    }

    bool operator==(const StringAtom &other) const {
      return value == other.value;
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
          if (std::holds_alternative<VariableID>(evaluated[0]) &&
              std::holds_alternative<VariableID>(evaluated[1])) {
            throw "Unimplemented case: At least one of the children must have a concrete value. "
                  "StringConstraint";
          } else if (std::holds_alternative<std::string>(evaluated[0]) &&
                     std::holds_alternative<VariableID>(evaluated[1])) {
            const auto assignedID = std::get<VariableID>(evaluated[1]);
            const auto &assignedString = std::get<std::string>(evaluated[0]);
            return assignIfPossible(env, assignedID, assignedString);
          } else if (std::holds_alternative<VariableID>(evaluated[0]) &&
                     std::holds_alternative<std::string>(evaluated[1])) {
            const auto assignedID = std::get<VariableID>(evaluated[0]);
            const auto &assignedString = std::get<std::string>(evaluated[1]);
            return assignIfPossible(env, assignedID, assignedString);
          } else {
            const auto &evaluated0 = std::get<std::string>(evaluated[0]);
            const auto &evaluated1 = std::get<std::string>(evaluated[1]);
            return evaluated0 == evaluated1;
          }
        }
        case kind_t::NE: {
          if (std::holds_alternative<VariableID>(evaluated[0]) &&
              std::holds_alternative<VariableID>(evaluated[1])) {
            throw "Unimplemented case: At least one of the children must have a concrete value. "
                  "StringConstraint";
          } else if (std::holds_alternative<std::string>(evaluated[0]) &&
                     std::holds_alternative<VariableID>(evaluated[1])) {
            return true;
          } else if (std::holds_alternative<VariableID>(evaluated[0]) &&
                     std::holds_alternative<std::string>(evaluated[1])) {
            return true;
          } else {
            const auto &evaluated0 = std::get<std::string>(evaluated[0]);
            const auto &evaluated1 = std::get<std::string>(evaluated[1]);
            return evaluated0 != evaluated1;
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
    explicit SCMaker(VariableID id) : first({id}) {
    }

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
} // namespace NonSymbolic
