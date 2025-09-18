/*!
 * @author Masaki Waga
 * @date 2019-01-24
 */

#ifndef DATAMONITOR_NON_SYMBOLIC_UPDATE_HH
#define DATAMONITOR_NON_SYMBOLIC_UPDATE_HH

#include <algorithm>
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

  struct Update {
    std::vector<std::pair<VariableID, VariableID>> stringUpdate;
    std::vector<std::pair<VariableID, VariableID>> numberUpdate;

    template <typename Number> void execute(StringValuation &stringEnv, NumberValuation<Number> &numEnv) const {
      for (const auto &update: stringUpdate) {
        const auto from = update.second;
        const auto to = update.first;
        stringEnv[to] = stringEnv[from];
      }
      for (const auto &update: numberUpdate) {
        const auto from = update.second;
        const auto to = update.first;
        numEnv[to] = numEnv[from];
      }
    }
  };
} // namespace NonSymbolic
#endif // DATAMONITOR_NON_SYMBOLIC_UPDATE_HH
