#pragma once

//(setq flycheck-clang-language-standard "c++17")

#include "automaton.hh"
#include "non_symbolic_update.hh"
#include "observer.hh"
#include "subject.hh"
#include "timed_word_subject.hh"
#include <boost/unordered_set.hpp>

template <class Number> struct BooleanMonitorResult {
  std::size_t index;
  double timestamp;
  NonSymbolic::NumberValuation<Number> numberValuation;
  NonSymbolic::StringValuation stringValuation;
};

namespace NonSymbolic {
  template <typename Number>
  class BooleanMonitor : public SingleSubject<BooleanMonitorResult<Number>>, public Observer<TimedWordEvent<Number>> {
  public:
    static const constexpr std::size_t unobservableActionID = 127;
    BooleanMonitor(const NonParametricTA<Number> &automaton) : automaton(automaton) {
      configurations.clear();
      // configurations.reserve(automaton.initialStates.size());
      std::vector<double> initCVal(automaton.clockVariableSize);
      // by default, initSEnv is no violating set (variant)
      StringValuation initSEnv(automaton.stringVariableSize);
      // by default, initNEnv is unset (optional)
      NumberValuation<Number> initNEnv(automaton.numberVariableSize);
      for (const auto &initialState: automaton.initialStates) {
        configurations.insert({initialState, initCVal, initSEnv, initNEnv, 0});
      }
    }
    virtual ~BooleanMonitor() {
      epsilonTransition(configurations);
    }
    void notify(const TimedWordEvent<Number> &event) {
      const Action actionId = event.actionId;
      const std::vector<std::string> &strings = event.strings;
      const std::vector<Number> &numbers = event.numbers;
      const double timestamp = event.timestamp;

      boost::unordered_set<Configuration> nextConfigurations;
      configurations.merge(epsilonTransition(configurations));

      for (const Configuration &conf: configurations) {
        // make the current env
        auto clockValuation = std::get<1>(conf); // conf.clockValuation;
        const auto absTime = std::get<4>(conf);
        if (timestamp < absTime) {
          continue;
        }
        for (double &d: clockValuation) {
          d += timestamp - absTime;
        }
        auto stringEnv = std::get<2>(conf); // conf.stringEnv;
        stringEnv.insert(stringEnv.end(), strings.begin(), strings.end());
        auto numberEnv = std::get<3>(conf); // conf.numberEnv;
        numberEnv.insert(numberEnv.end(), numbers.begin(), numbers.end());

        auto transitionIt = std::get<0>(conf)->next.find(actionId); // conf.state->next.find(actionId);
        if (transitionIt == std::get<0>(conf)->next.end()           // ;conf.state->next.end()
        ) {
          continue;
        }
        for (const auto &transition: transitionIt->second) {
          // evaluate the guards
          auto nextSEnv = stringEnv;
          if (eval(clockValuation, transition.guard) &&
              eval(transition.stringConstraints, nextSEnv, transition.numConstraints, numberEnv)) {
            auto nextCVal = clockValuation;
            auto nextNEnv = numberEnv;
            for (const VariableID resetVar: transition.resetVars) {
              nextCVal[resetVar] = 0;
            }
            transition.update.execute(nextSEnv, nextNEnv);
            nextSEnv.resize(automaton.stringVariableSize);
            nextNEnv.resize(automaton.numberVariableSize);
            auto target = transition.target.lock();
            if (!target) {
              continue;
            }
            nextConfigurations.insert({target, std::move(nextCVal), nextSEnv, nextNEnv, timestamp});
            if (target->isMatch) {
              this->notifyObservers({index, timestamp, nextNEnv, nextSEnv});
            }
          }
        }
      }
      index++;
      configurations = std::move(nextConfigurations);
    }

  private:
    const NonParametricTA<Number> automaton;
    using Configuration = std::tuple<std::shared_ptr<NonParametricTAState<Number>>, std::vector<double>,
                                     StringValuation, NumberValuation<Number>, double>;
    // struct Configuration {
    //   std::shared_ptr<AutomatonState<Number>> state;
    //   std::vector<double> clockValuation;
    //   StringValuation stringEnv;
    //   NumberValuation<Number> numberEnv;
    //   bool operator==(const Configuration x) const {
    //     return state == x.state && clockValuation == x.clockValuation && stringEnv == x.stringEnv && numberEnv ==
    //     x.numberEnv;
    //   }
    // };
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
          for (const auto &transition: transitionIt->second) {
            // evaluate the guards
            auto nextCVal = clockValuation;
            auto nextSEnv = stringEnv;
            auto nextNEnv = numberEnv;
            auto extendedGuard = transition.guard;

            auto absTime = std::get<4>(conf);
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
              auto nextState = transition.target.lock();
              transition.update.execute(nextSEnv, nextNEnv);
              nextConfigurations.insert({nextState, nextCVal, nextSEnv, nextNEnv, absTime});
              returnConfigurations.insert({nextState, nextCVal, nextSEnv, nextNEnv, absTime});
              if (nextState->isMatch) {
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
} // namespace NonSymbolic
