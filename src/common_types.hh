#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <vector>

using Action = std::size_t;
using ClockVariables = std::size_t;
using VariableID = std::size_t;

/*!
  @brief An automaton
 */
template <class State> struct Automaton {
  //! @brief The states of this automaton.
  std::vector<std::shared_ptr<State>> states;
  //! @brief The initial states of this automaton.
  std::vector<std::shared_ptr<State>> initialStates;

  //! @brief Returns the number of the states.
  inline std::size_t stateSize() const {
    return states.size();
  }

  inline bool operator==(const Automaton<State> &A) const {
    return initialStates == A.initialStates && states == A.states;
  }

  /*!
   * @brief Creates a deep copy of this automaton
   * @return A new Automaton instance that is a deep copy of this automaton
   */
  Automaton<State> deepCopy() const {
    Automaton<State> result;

    // Create a mapping from original states to their copies
    std::unordered_map<std::shared_ptr<State>, std::shared_ptr<State>> stateMap;

    // First, create copies of all states (without transitions)
    result.states.reserve(states.size());
    for (const auto &state: states) {
      auto stateCopy = std::make_shared<State>(state->isMatch);
      result.states.push_back(stateCopy);
      stateMap[state] = stateCopy;
    }

    // Set up initial states
    result.initialStates.reserve(initialStates.size());
    for (const auto &initialState: initialStates) {
      result.initialStates.push_back(stateMap[initialState]);
    }

    // Now copy all transitions, updating the target pointers
    for (size_t i = 0; i < states.size(); ++i) {
      const auto &originalState = states[i];
      auto &copiedState = result.states[i];

      // Copy the transitions map
      for (const auto &[action, transitions]: originalState->next) {
        auto &copiedTransitions = copiedState->next[action];
        copiedTransitions.reserve(transitions.size());

        for (const auto &transition: transitions) {
          // Create a copy of the transition
          auto transitionCopy = transition;

          // Update the target pointer to point to the corresponding copied state
          auto targetState = transition.target.lock();
          if (targetState && stateMap.find(targetState) != stateMap.end()) {
            transitionCopy.target = stateMap[targetState];
          }

          // Add the copied transition to the copied state
          copiedTransitions.push_back(std::move(transitionCopy));
        }
      }
    }

    return result;
  }
};

template <typename, typename = void> struct hasStringUpdate : std::false_type {};

template <typename T>
struct hasStringUpdate<T, std::void_t<decltype(std::declval<T>().stringUpdate)>> : std::true_type {};

template <typename, typename = void> struct hasNumberUpdate : std::false_type {};

template <typename T>
struct hasNumberUpdate<T, std::void_t<decltype(std::declval<T>().numberUpdate)>> : std::true_type {};

template <typename Update> struct UpdateTraits {
  static_assert(hasStringUpdate<Update>::value, "Update must have a member ``stringUpdate''.");
  static_assert(hasNumberUpdate<Update>::value, "Update must have a member ``numberUpdate''.");

  static decltype(std::declval<Update>().stringUpdate) &stringUpdate(Update &update) {
    return update.stringUpdate;
  }
  static decltype(std::declval<Update>().numberUpdate) &numberUpdate(Update &update) {
    return update.numberUpdate;
  }
};

template <typename, typename = void> struct hasRelease : std::false_type {};

template <typename T>
struct hasRelease<T, std::void_t<decltype(std::declval<T>().release(std::declval<std::size_t>()))>> : std::true_type {};
