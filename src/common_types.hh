#pragma once

#include <cstdlib>
#include <cstdint>
#include <memory>
#include <vector>

using Action = std::size_t;
using ClockVariables = std::size_t;
using VariableID = std::size_t;

/*!
  @brief An automaton
 */
template<class State>
struct Automaton {
  //! @brief The states of this automaton.
  std::vector<std::shared_ptr<State>> states;
  //! @brief The initial states of this automaton.
  std::vector<std::shared_ptr<State>> initialStates;

  //! @brief Returns the number of the states.
  inline std::size_t stateSize() const {return states.size ();}

  inline bool operator == (const Automaton<State> &A) const {
    return initialStates == A.initialStates &&
      states == A.states;
  }
};

template<typename, typename = void>
struct hasStringUpdate : std::false_type {};

template<typename T>
struct hasStringUpdate<T, std::void_t<decltype(std::declval<T>().stringUpdate)>> : std::true_type {};

template<typename, typename = void>
struct hasNumberUpdate : std::false_type {};

template<typename T>
struct hasNumberUpdate<T, std::void_t<decltype(std::declval<T>().numberUpdate)>> : std::true_type {};

template<typename Update>
struct UpdateTraits {
  static_assert(hasStringUpdate<Update>::value, "Update must have a member ``stringUpdate''.");
  static_assert(hasNumberUpdate<Update>::value, "Update must have a member ``numberUpdate''.");

  static decltype(std::declval<Update>().stringUpdate)& stringUpdate(Update& update) {
    return update.stringUpdate;
  }
  static decltype(std::declval<Update>().numberUpdate)& numberUpdate(Update& update) {
    return update.numberUpdate;
  }
};

template<typename, typename = void>
struct hasRelease : std::false_type {};

template<typename T>
struct hasRelease<T, std::void_t<decltype(std::declval<T>().release(std::declval<std::size_t>()))>> : std::true_type {};