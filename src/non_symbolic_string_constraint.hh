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

    void eval(const StringValuation &env, std::optional<std::string> &result) const {
      if (std::holds_alternative<std::string>(value)) {
        result = std::get<std::string>(value);
      } else if (env.at(std::get<std::size_t>(value))) {
        result = *env.at(std::get<std::size_t>(value));
      } else {
        result = std::nullopt;
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
      std::array<std::optional<std::string>, 2> evaluated;
      for (int i = 0; i < 2; i++) {
        children[i].eval(env, evaluated[i]);
      }
      switch (kind) {
        case kind_t::EQ:
          return *evaluated[0] == *evaluated[1];
        case kind_t::NE:
            return *evaluated[0] != *evaluated[1];
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
