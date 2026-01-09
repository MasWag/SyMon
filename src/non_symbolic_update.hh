/*!
 * @author Masaki Waga
 * @date 2019-01-24
 */

#ifndef DATAMONITOR_NON_SYMBOLIC_UPDATE_HH
#define DATAMONITOR_NON_SYMBOLIC_UPDATE_HH

#include <algorithm>
#include <optional>
#include <vector>

#include "non_symbolic_number_constraint.hh"
#include "non_symbolic_string_constraint.hh"

namespace NonSymbolic {
  template <typename Number>
  bool eval(const std::vector<StringConstraint> &stringConstraints, StringValuation &stringEnv,
            const std::vector<NumberConstraint<Number>> &numConstraints, const NumberValuation<Number> &numEnv) {
    return std::all_of(stringConstraints.begin(), stringConstraints.end(),
                       [&stringEnv](const StringConstraint &constraint) { return constraint.eval(stringEnv); }) &&
           std::all_of(numConstraints.begin(), numConstraints.end(),
                       [&numEnv](const NumberConstraint<Number> &constraint) { return constraint.eval(numEnv); });
  }

  template <typename Number>
  struct Update {
    std::vector<std::pair<VariableID, NonSymbolic::StringAtom>> stringUpdate;
    std::vector<std::pair<VariableID, NonSymbolic::NumberExpression<Number>>> numberUpdate;

    void execute(StringValuation &stringEnv, NumberValuation<Number> &numEnv) const {
      for (const auto &update: stringUpdate) {
        const auto from = update.second;
        const auto to = update.first;
        std::variant<VariableID, std::string> result;
        from.eval(stringEnv, result);
        std::optional<std::string> opt = std::nullopt;
        if (std::holds_alternative<std::string>(result)) {
          opt = std::get<std::string>(result);
        }
        stringEnv[to] = opt;
      }
      for (const auto &update: numberUpdate) {
        const auto from = update.second;
        const auto to = update.first;
        std::optional<Number> result;
        from.eval(numEnv, result);
        numEnv[to] = result;
      }
    }
  };
} // namespace NonSymbolic
#endif // DATAMONITOR_NON_SYMBOLIC_UPDATE_HH
