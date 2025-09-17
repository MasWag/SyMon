#pragma once

//(setq flycheck-clang-language-standard "c++17")

#include "subject.hh"
#include "observer.hh"
#include "timed_word_subject.hh"
#include "automaton.hh"
#include "non_symbolic_update.hh"
#include <boost/unordered_set.hpp>

template<class Number>
struct BooleanMonitorResult {
  std::size_t index;
  double timestamp;
  NonSymbolic::NumberValuation<Number> numberValuation;
  NonSymbolic::StringValuation stringValuation;
};

namespace NonSymbolic {
  template<typename Number>
  class BooleanMonitor : public SingleSubject<BooleanMonitorResult<Number>>,
                         public Observer<TimedWordEvent<Number>> {
  public:
    BooleanMonitor(const NonParametricTA<Number> &automaton) : automaton(automaton) {
      configurations.clear();
      // configurations.reserve(automaton.initialStates.size());
      std::vector<double> initCVal(automaton.clockVariableSize);
      // by default, initSEnv is no violating set (variant)
      StringValuation initSEnv(automaton.stringVariableSize);
      // by default, initNEnv is unset (optional)
      NumberValuation<Number> initNEnv(automaton.numberVariableSize);
      for (const auto &initialState: automaton.initialStates) {
        configurations.insert({initialState, initCVal, initSEnv, initNEnv});
      }
    }
    virtual ~BooleanMonitor() = default;
    void notify(const TimedWordEvent<Number>& event) {
      const Action actionId = event.actionId;
      const std::vector<std::string> &strings = event.strings;
      const std::vector<Number> &numbers = event.numbers;
      const double timestamp = event.timestamp;
      boost::unordered_set<Configuration> nextConfigurations;      
      for (const Configuration& conf: configurations) {
        // make the current env
        auto clockValuation = std::get<1>(conf);//conf.clockValuation;
        for (double& d: clockValuation) {
          d += timestamp - absTime;
        }
        auto stringEnv = std::get<2>(conf);//conf.stringEnv;
        stringEnv.insert(stringEnv.end(), strings.begin(), strings.end());
        auto numberEnv = std::get<3>(conf);//conf.numberEnv;
        numberEnv.insert(numberEnv.end(), numbers.begin(), numbers.end());

        auto transitionIt = std::get<0>(conf)->next.find(actionId);//conf.state->next.find(actionId);
        if (transitionIt == std::get<0>(conf)->next.end()// ;conf.state->next.end()
            ) {
          continue;
        }
        for (const auto& transition: transitionIt->second) {
          // evaluate the guards
          auto nextSEnv = stringEnv;
          if (eval(clockValuation, transition.guard) &&
              eval(transition.stringConstraints, nextSEnv,
                   transition.numConstraints, numberEnv)) {
            auto nextCVal = clockValuation;
            auto nextNEnv = numberEnv;
            for (const VariableID resetVar: transition.resetVars) {
              nextCVal[resetVar] = 0;
            }
            transition.update.execute(nextSEnv, nextNEnv);
            nextSEnv.resize(automaton.stringVariableSize);
            nextNEnv.resize(automaton.numberVariableSize);
            nextConfigurations.insert({transition.target.lock(),
                                       std::move(nextCVal),
                                       nextSEnv,
                                       nextNEnv});
            if (transition.target.lock()->isMatch) {
              this->notifyObservers({index, timestamp, nextNEnv, nextSEnv});
            }
          }
        }
      }
      absTime = timestamp;
      index++;
      configurations = std::move(nextConfigurations);
    }
  private:
    const NonParametricTA<Number> automaton;
    using Configuration = std::tuple<std::shared_ptr<NonParametricTAState<Number>>,
                                     std::vector<double>,
                                     StringValuation,
                                     NumberValuation<Number>>;
    // struct Configuration {
    //   std::shared_ptr<AutomatonState<Number>> state;
    //   std::vector<double> clockValuation;
    //   StringValuation stringEnv;
    //   NumberValuation<Number> numberEnv;
    //   bool operator==(const Configuration x) const {
    //     return state == x.state && clockValuation == x.clockValuation && stringEnv == x.stringEnv && numberEnv == x.numberEnv;
    //   }
    // };
    boost::unordered_set<Configuration> configurations;
    double absTime;
    std::size_t index = 0;
  };
}
