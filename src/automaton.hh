#pragma once

#include <vector>
#include <boost/unordered_map.hpp>
#include <memory>

#include "common_types.hh"
#include "timing_constraint.hh"
#include "parametric_timing_constraint.hh"

template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
struct AutomatonState;

/*!
  @brief A transition of a timed automaton
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update, typename = void>
struct AutomatonTransition {
  std::vector<StringConstraint> stringConstraints;
  std::vector<NumberConstraint> numConstraints;
  Update update;
  //! @brief The clock variables reset after this transition.
  std::vector<VariableID> resetVars;
  //! @brief The guard for this transition.
  TimingConstraint guard;
  std::weak_ptr<AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update>> target;
};

/*!
 * @brief A transition of a timed automaton when both string and number constraints have release
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
struct AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update,
        std::enable_if<std::conjunction<hasRelease<StringConstraint>, hasRelease<NumberConstraint>>::value>>
        : public AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update, bool> {
  //! @brief The string variables released after this transition.
  std::vector<VariableID> stringReleaseVars;
  //! @brief The number variables released after this transition.
  std::vector<VariableID> numReleaseVars;
};

/*!
  @brief A state of a timed automaton
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
struct AutomatonState {
  //! @brief The value is true if and only if the state is an accepting state.
  bool isMatch;
  /*!
    @brief An mapping of a character to the transitions.
    @note Because of non-determinism, the second element is a vector.
   */
  boost::unordered_map<Action, std::vector<AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update>>> next;

  explicit AutomatonState(const bool &isMatch) : isMatch(isMatch) {
    next.clear();
  }

  AutomatonState(const bool &isMatch,
                 const boost::unordered_map<Action, std::vector<AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update>>> &next)
          : isMatch(isMatch), next(std::move(next)) {}
};


/*!
  @brief A timed automaton
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
struct TimedAutomaton : public Automaton<AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update>> {
  std::size_t stringVariableSize, numberVariableSize, clockVariableSize;

  /*!
   * @brief Creates a deep copy of this timed automaton
   * @return A new TimedAutomaton instance that is a deep copy of this automaton
   */
  TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> deepCopy() const {
    using BaseAutomaton = Automaton<AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update>>;

    // Create a new TimedAutomaton
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> result;

    // Copy the base class members using the base class deepCopy method
    auto baseAutomatonCopy = BaseAutomaton::deepCopy();
    result.states = std::move(baseAutomatonCopy.states);
    result.initialStates = std::move(baseAutomatonCopy.initialStates);

    // Copy the TimedAutomaton specific members
    result.stringVariableSize = this->stringVariableSize;
    result.numberVariableSize = this->numberVariableSize;
    result.clockVariableSize = this->clockVariableSize;

    return result;
  }
};

template<typename StringConstraint, typename NumberConstraint, typename Update>
struct TimedAutomaton<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update>
        : public Automaton<AutomatonState<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update>> {
  std::size_t stringVariableSize, numberVariableSize, clockVariableSize, parameterSize;

  /*!
   * @brief Creates a deep copy of this parametric timed automaton
   * @return A new TimedAutomaton instance that is a deep copy of this automaton
   */
  TimedAutomaton<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> deepCopy() const {
    using BaseAutomaton = Automaton<AutomatonState<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update>>;

    // Create a new TimedAutomaton
    TimedAutomaton<StringConstraint, NumberConstraint, ParametricTimingConstraint, Update> result;

    // Copy the base class members using the base class deepCopy method
    auto baseAutomatonCopy = BaseAutomaton::deepCopy();
    result.states = std::move(baseAutomatonCopy.states);
    result.initialStates = std::move(baseAutomatonCopy.initialStates);

    // Copy the TimedAutomaton specific members
    result.stringVariableSize = this->stringVariableSize;
    result.numberVariableSize = this->numberVariableSize;
    result.clockVariableSize = this->clockVariableSize;
    result.parameterSize = this->parameterSize;

    return result;
  }
};

#include "non_symbolic_string_constraint.hh"
#include "non_symbolic_number_constraint.hh"
#include "non_symbolic_update.hh"

template<typename Number>
using NonParametricTA = TimedAutomaton<NonSymbolic::StringConstraint, NonSymbolic::NumberConstraint<Number>, std::vector<TimingConstraint>, NonSymbolic::Update>;
template<typename Number>
using NonParametricTAState = AutomatonState<NonSymbolic::StringConstraint, NonSymbolic::NumberConstraint<Number>, std::vector<TimingConstraint>, NonSymbolic::Update>;

#include "symbolic_string_constraint.hh"
#include "symbolic_number_constraint.hh"
#include "symbolic_update.hh"

using DataParametricTA = TimedAutomaton<Symbolic::StringConstraint, Symbolic::NumberConstraint, std::vector<TimingConstraint>, Symbolic::Update>;
using DataParametricTAState = AutomatonState<Symbolic::StringConstraint, Symbolic::NumberConstraint, std::vector<TimingConstraint>, Symbolic::Update>;

#include "parametric_timing_constraint.hh"

using ParametricTA = TimedAutomaton<Symbolic::StringConstraint, Symbolic::NumberConstraint, ParametricTimingConstraint, Symbolic::Update>;
using PTAState = AutomatonState<Symbolic::StringConstraint, Symbolic::NumberConstraint, ParametricTimingConstraint, Symbolic::Update>;
