#pragma once

#include "subject.hh"
#include "observer.hh"
#include "timed_word_subject.hh"
#include "automaton.hh"
#include "symbolic_update.hh"
#include "symbolic_number_constraint.hh"
#include "symbolic_string_constraint.hh"
#include "parametric_timing_constraint.hh"

#include <boost/unordered_set.hpp>

struct ParametricMonitorResult {
  std::size_t index;
  Parma_Polyhedra_Library::Coefficient timestamp;
  Symbolic::NumberValuation numberValuation;
  Symbolic::StringValuation stringValuation;
  ParametricTimingValuation parametricTimingValuation;
};

/*!
 * @note The Automaton can have unobservable transitions, but we assume that there is no loop of unobservable transitions.
 * @note The label of the unobservable events is 127 (This will be modified in a future version).
 * @note If the last trantision is an unobservable transition, the timestamp is that of the latest event.
 */
class ParametricMonitor : public SingleSubject<ParametricMonitorResult>,
                          public Observer<TimedWordEvent<Parma_Polyhedra_Library::Coefficient, Parma_Polyhedra_Library::Coefficient>> {
public:
  static const constexpr std::size_t unobservableActinoID = 127;

  explicit ParametricMonitor(const ParametricTA &automaton) : automaton(automaton) {
    absTime = 0;
    configurations.clear();
    // 1 -- |P|: Parameters, |P| + 1 -- |P| + |C|: Clocks
    ParametricTimingValuation initCVal(automaton.parameterSize);
    initCVal.add_space_dimensions_and_project(automaton.clockVariableSize);
    for (std::size_t i = 0; i < automaton.parameterSize; i++) {
      initCVal.add_constraint(Parma_Polyhedra_Library::Variable(i) >= 0);
    }
    // by default, initSEnv is no violating set (variant)
    Symbolic::StringValuation initSEnv(automaton.stringVariableSize);
    // by default, initNEnv is the universe of dimension automaton.numberVariableSize
    Symbolic::NumberValuation initNEnv(automaton.numberVariableSize);
    for (const auto &initialState: automaton.initialStates) {
      configurations.insert({initialState, initCVal, initSEnv, initNEnv});
    }
    elapsePolyhedron = Parma_Polyhedra_Library::NNC_Polyhedron(automaton.parameterSize +
                                                               automaton.clockVariableSize + 1);
    for (std::size_t i = 0; i < automaton.parameterSize; i++) {
      elapsePolyhedron.add_constraint(Parma_Polyhedra_Library::Variable(i) == 0);
    }
    for (std::size_t i = automaton.parameterSize; i <= automaton.parameterSize + automaton.clockVariableSize; i++) {
      elapsePolyhedron.add_constraint(Parma_Polyhedra_Library::Variable(i) == 1);
    }
  }

  /*
   * @note it tries unobservable transitions after the last event.
   */
  virtual ~ParametricMonitor() {
    boost::unordered_set<Configuration> nextConfigurations;

    boost::unordered_set<Configuration> currentConfigurations;
    for (Configuration conf: configurations) {
      // add a new dimension for time elapse.
      std::get<1>(conf).add_space_dimensions_and_project(1);
      currentConfigurations.insert(std::move(conf));
    }
    // Try unobservable transitions
    while (!currentConfigurations.empty()) {
      nextConfigurations.clear();
      for (const Configuration &conf: currentConfigurations) {
        auto transitionIt = std::get<0>(conf)->next.find(unobservableActinoID);
        if (transitionIt == std::get<0>(conf)->next.end()) {
          continue;
        }
        // make the current env
        auto clockValuation = std::get<1>(conf);
        assert(clockValuation.space_dimension() == automaton.parameterSize + automaton.clockVariableSize + 1);
        clockValuation.time_elapse_assign(elapsePolyhedron);
        const auto stringEnv = std::get<2>(conf);
        const auto numberEnv = std::get<3>(conf);
        for (const auto &transition: transitionIt->second) {
          // evaluate the guards
          auto nextCVal = clockValuation;
          auto nextSEnv = stringEnv;
          auto nextNEnv = numberEnv;
          auto extendedGuard = transition.guard;
          extendedGuard.add_space_dimensions_and_embed(1);
          if (eval(nextCVal, extendedGuard) &&
              eval(transition.stringConstraints, nextSEnv,
                   transition.numConstraints, nextNEnv)) {
            for (const VariableID resetVar: transition.resetVars) {
              nextCVal.affine_image(Parma_Polyhedra_Library::Variable(automaton.parameterSize + resetVar),
                                    Parma_Polyhedra_Library::Linear_Expression(0));
            }
            transition.update.execute(nextSEnv, nextNEnv);
            nextConfigurations.insert({transition.target.lock(),
                                       nextCVal,
                                       nextSEnv,
                                       nextNEnv});
            nextCVal.remove_higher_space_dimensions(automaton.parameterSize + automaton.clockVariableSize);
            if (transition.target.lock()->isMatch) {
              notifyObservers({index, absTime, nextNEnv, nextSEnv, nextCVal});
            }
          }
        }
      }

      std::swap(currentConfigurations, nextConfigurations);
    }
  }

  void
  notify(const TimedWordEvent<Parma_Polyhedra_Library::Coefficient, Parma_Polyhedra_Library::Coefficient> &event) override {
    const Action actionId = event.actionId;
    const std::vector<std::string> &strings = event.strings;
    const std::vector<Parma_Polyhedra_Library::Coefficient> &numbers = event.numbers;
    const Parma_Polyhedra_Library::Coefficient timestamp = event.timestamp;
    const auto dwellTime = timestamp - absTime;
    boost::unordered_set<Configuration> nextConfigurations;

    boost::unordered_set<Configuration> currentConfigurations;
    for (Configuration conf: configurations) {
      // add a new dimension for time elapse.
      std::get<1>(conf).add_space_dimensions_and_project(1);
      currentConfigurations.insert(std::move(conf));
    }
    // time elapse
    for (Configuration conf: configurations) {
      for (std::size_t i = 0; i < automaton.clockVariableSize; i++) {
        //! @todo Currently, the timestamp is mpz (integer). I will make it mpq (quadratic) later.
        std::get<1>(conf).affine_image(Parma_Polyhedra_Library::Variable(automaton.parameterSize + i),
                                       Parma_Polyhedra_Library::Variable(automaton.parameterSize + i) + dwellTime);
      }
      nextConfigurations.insert(std::move(conf));
    }
    std::swap(configurations, nextConfigurations);

    // Try unobservable transitions
    while (!currentConfigurations.empty()) {
      nextConfigurations.clear();
      for (const Configuration &conf: currentConfigurations) {
        auto transitionIt = std::get<0>(conf)->next.find(unobservableActinoID);
        if (transitionIt == std::get<0>(conf)->next.end()) {
          continue;
        }
        // make the current env
        auto clockValuation = std::get<1>(conf);
        clockValuation.time_elapse_assign(elapsePolyhedron);
        clockValuation.add_constraint(
                Parma_Polyhedra_Library::Variable(automaton.parameterSize + automaton.clockVariableSize) <= dwellTime);
        const auto stringEnv = std::get<2>(conf);
        const auto numberEnv = std::get<3>(conf);
        for (const auto &transition: transitionIt->second) {
          // evaluate the guards
          auto nextCVal = clockValuation;
          auto nextSEnv = stringEnv;
          auto nextNEnv = numberEnv;
          auto extendedGuard = transition.guard;
          extendedGuard.add_space_dimensions_and_embed(1);
          if (eval(nextCVal, extendedGuard) &&
              eval(transition.stringConstraints, nextSEnv,
                   transition.numConstraints, nextNEnv)) {
            for (const VariableID resetVar: transition.resetVars) {
              nextCVal.affine_image(Parma_Polyhedra_Library::Variable(automaton.parameterSize + resetVar),
                                    Parma_Polyhedra_Library::Linear_Expression(0));
            }
            transition.update.execute(nextSEnv, nextNEnv);
            nextConfigurations.insert({transition.target.lock(),
                                       nextCVal,
                                       nextSEnv,
                                       nextNEnv});
            if (transition.target.lock()->isMatch) {
              auto tmpNCV = nextCVal;
              tmpNCV.remove_higher_space_dimensions(automaton.parameterSize + automaton.clockVariableSize);
              notifyObservers({index, absTime, nextNEnv, nextSEnv, tmpNCV});
            }
            // time elapse
            for (std::size_t i = 0; i < automaton.clockVariableSize; i++) {
              //! @todo Currently, the timestamp is mpz (integer). I will make it mpq (quadratic) later.
              nextCVal.affine_image(Parma_Polyhedra_Library::Variable(automaton.parameterSize + i),
                                    Parma_Polyhedra_Library::Variable(automaton.parameterSize + i) + dwellTime -
                                    Parma_Polyhedra_Library::Variable(
                                            automaton.parameterSize + automaton.clockVariableSize));
            }
            nextCVal.remove_higher_space_dimensions(automaton.parameterSize + automaton.clockVariableSize);
            configurations.insert({transition.target.lock(),
                                   nextCVal,
                                   nextSEnv,
                                   nextNEnv});
          }
        }
      }

      std::swap(currentConfigurations, nextConfigurations);
    }

    nextConfigurations.clear();
    boost::unordered_map<std::tuple<std::shared_ptr<PTAState>,
            ParametricTimingValuation,
            Symbolic::StringValuation>,
            Parma_Polyhedra_Library::Pointset_Powerset<Symbolic::NumberValuation>> mergedConfigurations;

    // Try observable transitions
    for (const Configuration &conf: configurations) {
      auto transitionIt = std::get<0>(conf)->next.find(actionId);
      if (transitionIt == std::get<0>(conf)->next.end()) {
        continue;
      }
      // make the current env
      // The time elapsed in the above
      auto clockValuation = std::get<1>(conf); //.clockValuation;
      auto stringEnv = std::get<2>(conf); //.stringEnv;
      stringEnv.insert(stringEnv.end(), strings.begin(), strings.end());
      auto numberEnv = std::get<3>(conf); //.numberEnv;
      // add dimension for the data in the timed word.
      assert(numberEnv.space_dimension() == automaton.numberVariableSize);
      numberEnv.add_space_dimensions_and_embed(numbers.size());
      for (std::size_t i = 0; i < numbers.size(); i++) {
        numberEnv.add_constraint(Parma_Polyhedra_Library::Variable(automaton.numberVariableSize + i) == numbers[i]);
      }
      for (const auto &transition: transitionIt->second) {
        // evaluate the guards
        auto nextCVal = clockValuation;
        auto nextSEnv = stringEnv;
        auto nextNEnv = numberEnv;
        if (eval(nextCVal, transition.guard) &&
            eval(transition.stringConstraints, nextSEnv,
                 transition.numConstraints, nextNEnv)) {
          for (const VariableID resetVar: transition.resetVars) {
            nextCVal.affine_image(Parma_Polyhedra_Library::Variable(automaton.parameterSize + resetVar),
                                  Parma_Polyhedra_Library::Linear_Expression(0));
          }
          transition.update.execute(nextSEnv, nextNEnv);
          nextSEnv.resize(automaton.stringVariableSize);
          nextNEnv.remove_higher_space_dimensions(automaton.numberVariableSize);
          const auto key = std::make_tuple(transition.target.lock(), nextCVal, nextSEnv);
          const auto it = mergedConfigurations.find(key);
          if (it == mergedConfigurations.end()) {
            mergedConfigurations[key] = Parma_Polyhedra_Library::Pointset_Powerset<Symbolic::NumberValuation>{nextNEnv};
          } else {
            it->second.add_disjunct(nextNEnv);
          }
          if (transition.target.lock()->isMatch) {
            notifyObservers({index, timestamp, nextNEnv, nextSEnv, nextCVal});
          }
        }
      }
    }
    absTime = timestamp;
    index++;
    boost::unordered_map<std::tuple<std::shared_ptr<PTAState>,
            ParametricTimingValuation,
            Symbolic::NumberValuation>,
            std::vector<Symbolic::StringValuation>> stringMergedConfigurations;
    //merge numberEnv
    configurations.clear();
    for (auto &conf: mergedConfigurations) {
      conf.second.pairwise_reduce();
      for (auto numberEnv: conf.second) {
        const auto key = std::make_tuple(std::get<0>(conf.first), std::get<1>(conf.first), numberEnv.pointset());
        auto it = stringMergedConfigurations.find(key);
        if (it == stringMergedConfigurations.end()) {
          stringMergedConfigurations[key] = {std::get<2>(conf.first)};
        } else {
          it->second.push_back(std::get<2>(conf.first));
        }
      }
    }

    //merge stringEnv
    configurations.clear();
    for (auto &conf: stringMergedConfigurations) {
      Symbolic::pairwise_reduce(conf.second);
      for (const auto& stringEnv: conf.second) {
        configurations.insert(std::make_tuple(std::get<0>(conf.first),
                                              std::get<1>(conf.first),
                                              stringEnv,
                                              std::get<2>(conf.first)));
      }
    }
  }

private:
  const ParametricTA automaton;
  using Configuration = std::tuple<std::shared_ptr<PTAState>,
          ParametricTimingValuation,
          Symbolic::StringValuation,
          Symbolic::NumberValuation>;
  //Symbolic::NumberValuation>;
/*  struct Configuration {
    std::shared_ptr<DataParametricTAState> state;
    std::vector<double> clockValuation;
    NonSymbolic::StringValuation stringEnv;
    Symbolic::NumberValuation numberEnv;
  };*/
  boost::unordered_set<Configuration> configurations;
  Parma_Polyhedra_Library::Coefficient absTime;
  std::size_t index = 0;
  Parma_Polyhedra_Library::NNC_Polyhedron elapsePolyhedron;
};
