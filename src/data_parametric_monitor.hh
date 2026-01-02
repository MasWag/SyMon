#pragma once

#include "automaton.hh"
#include "observer.hh"
#include "ppl_rational.hh"
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
                              public Observer<TimedWordEvent<PPLRational>> {
public:
  static const constexpr std::size_t unobservableActionID = 127;
  explicit DataParametricMonitor(const DataParametricTA &automaton) : automaton(automaton) {
    configurations.clear();
    // configurations.reserve(automaton.initialStates.size());
    std::vector<double> initCVal(automaton.clockVariableSize);
    // by default, initSEnv is no violating set (variant)
    Symbolic::StringValuation initSEnv(automaton.stringVariableSize);
    // by default, initNEnv is the universe of dimension automaton.numberVariableSize
    Symbolic::NumberValuation initNEnv(automaton.numberVariableSize);
    for (const auto &initialState: automaton.initialStates) {
      configurations.insert({initialState, initCVal, initSEnv, initNEnv, 0});
    }
  }

  virtual ~DataParametricMonitor() {
    epsilonTransition(configurations);
  }

  void notify(const TimedWordEvent<PPLRational> &event) override {
    const Action actionId = event.actionId;
    const std::vector<std::string> &strings = event.strings;
    const std::vector<PPLRational> &numbers = event.numbers;
    const double timestamp = event.timestamp;

    boost::unordered_set<Configuration> nextConfigurations;
    configurations.merge(epsilonTransition(configurations));

    for (const Configuration &conf: configurations) {
      // make the current env
      auto clockValuation = std::get<1>(conf); //.clockValuation;
      const auto absTime = std::get<4>(conf);
      if (timestamp < absTime) {
        continue;
      }
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
        numberEnv.add_constraint(Parma_Polyhedra_Library::Variable(automaton.numberVariableSize + i) * numbers[i].getDenominator() == numbers[i].getNumerator());
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
          nextConfigurations.insert({transition.target.lock(), std::move(nextCVal), nextSEnv, nextNEnv, timestamp});
          if (transition.target.lock()->isMatch) {
            notifyObservers({index, timestamp, nextNEnv, nextSEnv});
          }
        }
      }
    }
    index++;
    configurations = std::move(nextConfigurations);
  }

private:
  const DataParametricTA automaton;
  using Configuration = std::tuple<std::shared_ptr<DataParametricTAState>, std::vector<double>,
                                   Symbolic::StringValuation, Symbolic::NumberValuation, double>;
  // Symbolic::NumberValuation>;
  /*  struct Configuration {
      std::shared_ptr<DataParametricTAState> state;
      std::vector<double> clockValuation;
      NonSymbolic::StringValuation stringEnv;
      Symbolic::NumberValuation numberEnv;
    };*/
  boost::unordered_set<Configuration> configurations;
  std::size_t index = 0;

  /**
   * Performs epsilon (unobservable) transitions starting from the given configurations.
   *
   * This method repeatedly explores transitions labeled with the unobservable action
   * from all current configurations, advancing time according to clock guards,
   * applying clock resets and symbolic updates, and collecting all reachable
   * configurations. Exploration continues until no further epsilon transitions
   * are possible. Whenever a target state marked as a match is reached,
   * it notifies registered observers using the current index, absolute time, and valuations.
   *
   * @param currentConfigurations
   *        The initial set of configurations from which epsilon transitions
   *        are taken. The set is passed by value and moved into an internal
   *        worklist; it should not be used by the caller after this call.
   *
   * @return The set of configurations reachable via zero or more epsilon
   *         transitions that cannot be further extended by additional epsilon
   *         transitions.
   *
   */
  boost::unordered_set<Configuration> epsilonTransition(boost::unordered_set<Configuration> currentConfigurations) {
    // the next configurations to explore
    boost::unordered_set<Configuration> nextConfigurations;
    // the configurations reachable via epsilon transitions
    boost::unordered_set<Configuration> returnConfigurations;

    while (!currentConfigurations.empty()) {
      nextConfigurations.clear();
      for (const Configuration &conf: currentConfigurations) {
        auto transitionIt = std::get<0>(conf)->next.find(unobservableActionID);
        if (transitionIt == std::get<0>(conf)->next.end()) {
          continue;
        }
        // make the current env
        const auto clockValuation = std::get<1>(conf);
        const auto stringEnv = std::get<2>(conf);
        const auto numberEnv = std::get<3>(conf);
        auto absTime = std::get<4>(conf);
        for (const auto &transition: transitionIt->second) {
          // evaluate the guards
          auto nextCVal = clockValuation;
          auto nextSEnv = stringEnv;
          auto nextNEnv = numberEnv;
          auto extendedGuard = transition.guard;
          
          auto df = diff(nextCVal, extendedGuard);
          if (!df) continue;
          for (double &d: nextCVal) {
            d += df.value();
          }
          absTime += df.value();

          if (eval(nextCVal, extendedGuard) &&
              eval(transition.stringConstraints, nextSEnv, transition.numConstraints, nextNEnv)) {
            for (const VariableID resetVar: transition.resetVars) {
              nextCVal[resetVar] = 0;
            }
            transition.update.execute(nextSEnv, nextNEnv);
            nextConfigurations.insert({transition.target.lock(), nextCVal, nextSEnv, nextNEnv, absTime});
            returnConfigurations.insert({transition.target.lock(), nextCVal, nextSEnv, nextNEnv, absTime});
            if (transition.target.lock()->isMatch) {
              this->notifyObservers({index, absTime, nextNEnv, nextSEnv});
            }
          }
        }
      }
      std::swap(currentConfigurations, nextConfigurations);
    }
    return returnConfigurations;
  }
};
