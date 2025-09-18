#pragma once

#include "automaton.hh"
#include "observer.hh"
#include "subject.hh"
#include "symbolic_number_constraint.hh"
#include "symbolic_string_constraint.hh"
#include "symbolic_update.hh"
#include "timed_word_subject.hh"

namespace Parma_Polyhedra_Library {
  static inline std::size_t hash_value(const Symbolic::NumberValuation &p) {
    return static_cast<std::size_t>(p.hash_code());
  }
} // namespace Parma_Polyhedra_Library

#include <boost/unordered_set.hpp>

struct DataParametricMonitorResult {
  std::size_t index;
  double timestamp;
  Symbolic::NumberValuation numberValuation;
  Symbolic::StringValuation stringValuation;
};

class DataParametricMonitor : public SingleSubject<DataParametricMonitorResult>,
                              public Observer<TimedWordEvent<Parma_Polyhedra_Library::Coefficient>> {
public:
  explicit DataParametricMonitor(const DataParametricTA &automaton) : automaton(automaton) {
    absTime = 0;
    configurations.clear();
    // configurations.reserve(automaton.initialStates.size());
    std::vector<double> initCVal(automaton.clockVariableSize);
    // by default, initSEnv is no violating set (variant)
    Symbolic::StringValuation initSEnv(automaton.stringVariableSize);
    // by default, initNEnv is the universe of dimension automaton.numberVariableSize
    Symbolic::NumberValuation initNEnv(automaton.numberVariableSize);
    for (const auto &initialState: automaton.initialStates) {
      configurations.insert({initialState, initCVal, initSEnv, initNEnv});
    }
  }

  virtual ~DataParametricMonitor() = default;

  void notify(const TimedWordEvent<Parma_Polyhedra_Library::Coefficient> &event) override {
    const Action actionId = event.actionId;
    const std::vector<std::string> &strings = event.strings;
    const std::vector<Parma_Polyhedra_Library::Coefficient> &numbers = event.numbers;
    const double timestamp = event.timestamp;
    boost::unordered_set<Configuration> nextConfigurations;

    for (const Configuration &conf: configurations) {
      // make the current env
      auto clockValuation = std::get<1>(conf); //.clockValuation;
      for (double &d: clockValuation) {
        d += timestamp - absTime;
      }
      auto stringEnv = std::get<2>(conf); //.stringEnv;
      stringEnv.insert(stringEnv.end(), strings.begin(), strings.end());
      auto numberEnv = std::get<3>(conf); //.numberEnv;
      // add dimension for the data in the timed word.
      assert(numberEnv.space_dimension() == automaton.numberVariableSize);
      numberEnv.add_space_dimensions_and_embed(numbers.size());
      for (std::size_t i = 0; i < numbers.size(); i++) {
        numberEnv.add_constraint(Parma_Polyhedra_Library::Variable(automaton.numberVariableSize + i) == numbers[i]);
      }

      auto transitionIt = std::get<0>(conf)->next.find(actionId);
      if (transitionIt == std::get<0>(conf)->next.end()) {
        continue;
      }
      for (const auto &transition: transitionIt->second) {
        // evaluate the guards
        auto nextSEnv = stringEnv;
        auto nextNEnv = numberEnv;
        if (eval(clockValuation, transition.guard) &&
            eval(transition.stringConstraints, nextSEnv, transition.numConstraints, nextNEnv)) {
          auto nextCVal = clockValuation;
          for (const VariableID resetVar: transition.resetVars) {
            nextCVal[resetVar] = 0;
          }
          transition.update.execute(nextSEnv, nextNEnv);
          nextSEnv.resize(automaton.stringVariableSize);
          nextNEnv.remove_higher_space_dimensions(automaton.numberVariableSize);
          nextConfigurations.insert({transition.target.lock(), std::move(nextCVal), nextSEnv, nextNEnv});
          if (transition.target.lock()->isMatch) {
            notifyObservers({index, timestamp, nextNEnv, nextSEnv});
          }
        }
      }
    }
    absTime = timestamp;
    index++;
    configurations = std::move(nextConfigurations);
  }

private:
  const DataParametricTA automaton;
  using Configuration = std::tuple<std::shared_ptr<DataParametricTAState>, std::vector<double>,
                                   Symbolic::StringValuation, Symbolic::NumberValuation>;
  // Symbolic::NumberValuation>;
  /*  struct Configuration {
      std::shared_ptr<DataParametricTAState> state;
      std::vector<double> clockValuation;
      NonSymbolic::StringValuation stringEnv;
      Symbolic::NumberValuation numberEnv;
    };*/
  boost::unordered_set<Configuration> configurations;
  double absTime;
  std::size_t index = 0;
};