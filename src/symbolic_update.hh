//
// Created by Masaki Waga on 2019-01-25.
//

#ifndef DATAMONITOR_SYMBOLIC_UPDATE_HH
#define DATAMONITOR_SYMBOLIC_UPDATE_HH

#include "symbolic_number_constraint.hh"
#include "symbolic_string_constraint.hh"

namespace Symbolic {
  struct Update {
    std::vector<std::pair<VariableID, Symbolic::StringAtom>> stringUpdate;
    std::vector<std::pair<VariableID, Symbolic::NumberExpression>> numberUpdate;

    void execute(Symbolic::StringValuation &stringEnv, Symbolic::NumberValuation &numEnv) const {
      for (const auto &update: stringUpdate) {
        const auto from = update.second;
        const auto to = update.first;
        std::variant<std::vector<std::string>, std::string> result;
        from.eval(stringEnv, result);
        stringEnv[to] = result;
      }
      for (const auto &update: numberUpdate) {
        const auto from = update.second;
        const auto to = update.first;
        Parma_Polyhedra_Library::Variable toVar(to);
        numEnv.affine_image(toVar, from);
        // numEnv.unconstrain(toVar);
        // Parma_Polyhedra_Library::TimingConstraint constraint = toVar == from;
        // numEnv.add_constraint(constraint);
      }
    }
  };

  static inline bool evalUpdate(const std::vector<Symbolic::NumberConstraint> &numConstraints,
                                Symbolic::NumberValuation &numEnv) {
    Parma_Polyhedra_Library::Constraint_System cs;
    for (const auto &numConstraint: numConstraints) {
      numEnv.add_constraint(numConstraint);
    }
    return !numEnv.is_empty();
  }

  static inline bool eval(const std::vector<Symbolic::StringConstraint> &stringConstraints,
                          Symbolic::StringValuation &stringEnv,
                          const std::vector<Symbolic::NumberConstraint> &numConstraints,
                          Symbolic::NumberValuation &numEnv) {
    return std::all_of(
               stringConstraints.begin(), stringConstraints.end(),
               [&stringEnv](const Symbolic::StringConstraint &constraint) { return constraint.eval(stringEnv); }) &&
           evalUpdate(numConstraints, numEnv);
  }
} // namespace Symbolic
#endif // DATAMONITOR_SYMBOLIC_UPDATE_HH
