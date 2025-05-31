#pragma once

#include "automaton.hh"

/*!
 * @brief Compute the disjunction of two timed automata.
 *
 * @param[in] left   The first (left-hand) timed automaton.
 * @param[in] right  The second (right-hand) timed automaton.
 * @return A TimedAutomaton representing the logical disjunction (union) of the two input automata.
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> disjunction(
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &&left,
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &&right) {
    // Juxtapose two automata
    left.clockVariableSize = std::max(left.clockVariableSize, right.clockVariableSize);
    left.stringVariableSize = std::max(left.stringVariableSize, right.stringVariableSize);
    left.numberVariableSize = std::max(left.numberVariableSize, right.numberVariableSize);
    left.states.reserve(left.states.size() + right.states.size());
    std::move(right.states.begin(), right.states.end(), std::back_inserter(left.states));
    left.initialStates.reserve(left.initialStates.size() + right.initialStates.size());
    std::move(right.initialStates.begin(), right.initialStates.end(), std::back_inserter(left.initialStates));

    return left;
}

/*!
 * @brief Compute the intersection of two timed automata.
 *
 * We take the intersection of two timed automata by taking a product of them.
 * @note We assume that the string and number variables and timing parameters are global,
 *       i.e., they are shared between the two automata. In contrast, the clock variables are local to each automaton.
 *
 * @param[in] left   The first (left-hand) timed automaton.
 * @param[in] right  The second (right-hand) timed automaton.
 * @return A TimedAutomaton representing the intersection of two automata.
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> conjunction(
    const TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &left,
    const TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &right) {
    using State = AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update>;
    using StatePtr = std::shared_ptr<State>;
    // Create a new automaton for the intersection
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> result;
    result.clockVariableSize = left.clockVariableSize + right.clockVariableSize;
    result.stringVariableSize = std::max(left.stringVariableSize, right.stringVariableSize);
    result.numberVariableSize = std::max(left.numberVariableSize, right.numberVariableSize);
    result.states.reserve(left.states.size() * right.states.size());
    result.initialStates.reserve(left.initialStates.size() * right.initialStates.size());
    // Create a map to hold the states of the product automaton
    std::unordered_map<State *, std::pair<StatePtr, StatePtr> > stateMap;
    boost::unordered_map<std::pair<State *, State *>, StatePtr> reverseStateMap;
    std::vector<StatePtr> waitingStates;
    waitingStates.reserve(left.states.size() * right.states.size());
    // Create the initial states of the product automaton
    for (const auto &leftInitial: left.initialStates) {
        for (const auto &rightInitial: right.initialStates) {
            auto newState = std::make_shared<State>(leftInitial->isMatch && rightInitial->isMatch);
            stateMap[newState.get()] = {leftInitial, rightInitial};
            reverseStateMap[std::make_pair(leftInitial.get(), rightInitial.get())] = newState;
            result.initialStates.push_back(newState);
            waitingStates.push_back(newState);
        }
    }

    // Process the waiting states
    while (!waitingStates.empty()) {
        auto currentState = waitingStates.back();
        waitingStates.pop_back();
        auto &[leftState, rightState] = stateMap[currentState.get()];

        // Create transitions for the product automaton
        for (const auto &[label, leftTransitions]: leftState->next) {
            for (const auto &leftTransition: leftTransitions) {
                auto it = rightState->next.find(label);
                if (it == rightState->next.end()) {
                    continue;
                }
                for (const auto &rightTransition: it->second) {
                    // Create a new transition in the product automaton
                    std::vector<StringConstraint> stringConstraints;
                    stringConstraints.reserve(leftTransition.stringConstraints.size() +
                                              rightTransition.stringConstraints.size());
                    std::copy(leftTransition.stringConstraints.begin(),
                              leftTransition.stringConstraints.end(),
                              std::back_inserter(stringConstraints));
                    std::copy(rightTransition.stringConstraints.begin(),
                              rightTransition.stringConstraints.end(),
                              std::back_inserter(stringConstraints));

                    std::vector<NumberConstraint> numConstraints;
                    numConstraints.reserve(leftTransition.numConstraints.size() +
                                           rightTransition.numConstraints.size());
                    std::copy(leftTransition.numConstraints.begin(),
                              leftTransition.numConstraints.end(),
                              std::back_inserter(numConstraints));
                    std::copy(rightTransition.numConstraints.begin(),
                              rightTransition.numConstraints.end(),
                              std::back_inserter(numConstraints));

                    Update update;
                    update.stringUpdate.reserve(leftTransition.update.stringUpdate.size() +
                                                rightTransition.update.stringUpdate.size());
                    std::copy(leftTransition.update.stringUpdate.begin(),
                              leftTransition.update.stringUpdate.end(),
                              std::back_inserter(update.stringUpdate));
                    std::copy(rightTransition.update.stringUpdate.begin(),
                              rightTransition.update.stringUpdate.end(),
                              std::back_inserter(update.stringUpdate));

                    update.numberUpdate.reserve(leftTransition.update.numberUpdate.size() +
                                                rightTransition.update.numberUpdate.size());
                    std::copy(leftTransition.update.numberUpdate.begin(),
                              leftTransition.update.numberUpdate.end(),
                              std::back_inserter(update.numberUpdate));
                    std::copy(rightTransition.update.numberUpdate.begin(),
                              rightTransition.update.numberUpdate.end(),
                              std::back_inserter(update.numberUpdate));

                    // Reset variables are combined from both transitions
                    std::vector<VariableID> resetVars;
                    resetVars.reserve(leftTransition.resetVars.size() +
                                      rightTransition.resetVars.size());
                    std::copy(leftTransition.resetVars.begin(), leftTransition.resetVars.end(),
                              std::back_inserter(resetVars));
                    for (const auto &var: rightTransition.resetVars) {
                        // Adjust the variable ID for the right automaton
                        resetVars.push_back(var + left.clockVariableSize);
                    }

                    // Create the guard for the transition
                    TimingConstraint guard = leftTransition.guard && shift(
                                                 rightTransition.guard, left.clockVariableSize);

                    // Check if the target state already exists
                    auto it2 = reverseStateMap.find(
                        std::make_pair(leftTransition.target.lock().get(), rightTransition.target.lock().get()));
                    if (it2 != reverseStateMap.end()) {
                        // If it exists, add the transition to the existing state
                        currentState->next[label].push_back({
                            std::move(stringConstraints), std::move(numConstraints), std::move(update),
                            std::move(resetVars), std::move(guard), it2->second
                        });
                    } else {
                        // If it does not exist, create a new state
                        auto newTargetState = std::make_shared<State>(
                            leftTransition.target.lock()->isMatch && rightTransition.target.lock()->isMatch);
                        stateMap[newTargetState.get()] = {leftTransition.target.lock(), rightTransition.target.lock()};
                        reverseStateMap[std::make_pair(leftTransition.target.lock().get(),
                                                       rightTransition.target.lock().get())] = newTargetState;
                        currentState->next[label].push_back({
                            std::move(stringConstraints), std::move(numConstraints), std::move(update),
                            std::move(resetVars), std::move(guard), newTargetState
                        });
                        waitingStates.push_back(newTargetState);
                    }
                }
            }
        }
    }

    return result;
}

/*!
 * @brief Compute the concatenation of two timed automata.
 *
 * @param[in] left   The first (left-hand) timed automaton.
 * @param[in] right  The second (right-hand) timed automaton.
 * @return A TimedAutomaton representing the concatenation of two automata.
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> concatenate(
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &&left,
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &&right) {
    // Concatenate two automata
    left.clockVariableSize = std::max(left.clockVariableSize, right.clockVariableSize);
    left.stringVariableSize = std::max(left.stringVariableSize, right.stringVariableSize);
    left.numberVariableSize = std::max(left.numberVariableSize, right.numberVariableSize);

    std::vector<std::shared_ptr<AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update> > >
            removedStates;
    removedStates.reserve(left.states.size());
    std::vector<std::shared_ptr<AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update> > >
            leftFinalStates;
    leftFinalStates.reserve(left.states.size());
    for (const auto &state: left.states) {
        if (state->isMatch) {
            leftFinalStates.push_back(state);
            // The left final state is removed if it has no successor
            if (state->next.empty()) {
                removedStates.push_back(state);
            }
        }
    }

    // For any transition to the left final state, we make a transition with the same meta data to the right initial state
    for (const auto &source: left.states) {
        for (auto &[label, transitions]: source->next) {
            std::vector<AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update> >
                    newTransitions;
            for (auto it = transitions.begin(); it != transitions.end();) {
                if (it->target.lock()->isMatch) {
                    for (auto &ri: right.initialStates) {
                        // Create a new transition to the initial state of the right automaton
                        // We need to reset all clock variables
                        std::vector<VariableID> resetVars;
                        resetVars.reserve(left.clockVariableSize);
                        for (VariableID i = 0; i < left.clockVariableSize; ++i) {
                            resetVars.push_back(i);
                        }
                        newTransitions.push_back({
                            it->stringConstraints, it->numConstraints, it->update,
                            resetVars, it->guard, ri
                        });
                    }
                    // If the target state has no successor, we remove it
                    if (it->target.lock()->next.empty()) {
                        it = transitions.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    ++it;
                }
            }
            // Add the new transitions to the left automaton
            transitions.reserve(transitions.size() + newTransitions.size());
            std::move(newTransitions.begin(), newTransitions.end(), std::back_inserter(transitions));
        }
    }

    // Make the left final states non-final
    for (const auto &state: leftFinalStates) {
        state->isMatch = false;
    }
    // Remove the unnecessary states
    for (const auto &state: removedStates) {
        left.states.erase(std::remove(left.states.begin(), left.states.end(), state), left.states.end());
    }

    // Move the right states to left
    left.states.reserve(left.states.size() + right.states.size());
    std::move(right.states.begin(), right.states.end(), std::back_inserter(left.states));

    return left;
}

/*!
 * @brief Compute the automaton that accepts the empty string or the words accepted by the given timed automaton.
 *
 * @param[in] given The given timed automaton.
 * @return A TimedAutomaton recognizing the empty string or the words accepted by the given automaton.
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> emptyOr(
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &&given) {
    // Create a new initial state that is also a final state and has no outgoing transitions.
    auto newInitialState = std::make_shared<AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update> >(true);
    given.states.push_back(newInitialState);
    given.initialStates.push_back(std::move(newInitialState));

    return given;
}

/*!
 * @brief Compute the Kleene-Plus of the given timed automaton.
 *
 * @param[in] given The given timed automaton.
 * @return A TimedAutomaton representing the concatenation of two automata.
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> plus(
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &&given) {
    // For each transition to the final state, we duplicate it and make a transition to the initial state
    for (const auto &sourceState: given.states) {
        for (auto &[label, transitions]: sourceState->next) {
            std::vector<AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update> >
                    newTransitions;

            for (const auto &transition: transitions) {
                if (transition.target.lock()->isMatch) {
                    // If this transition leads to a final state, duplicate it to also lead to all initial states
                    for (auto &initialState: given.initialStates) {
                        // Create a new transition to the initial state
                        // We need to reset all clock variables
                        std::vector<VariableID> resetVars;
                        resetVars.reserve(given.clockVariableSize);
                        for (VariableID i = 0; i < given.clockVariableSize; ++i) {
                            resetVars.push_back(i);
                        }
                        newTransitions.push_back({
                            transition.stringConstraints,
                            transition.numConstraints,
                            transition.update,
                            resetVars,
                            transition.guard,
                            initialState
                        });
                    }
                }
            }

            // Add the new transitions
            transitions.reserve(transitions.size() + newTransitions.size());
            std::move(newTransitions.begin(), newTransitions.end(), std::back_inserter(transitions));
        }
    }

    return given;
}

/*!
 * @brief Compute the Kleene-Star of the given timed automaton.
 *
 * @param[in] given The given timed automaton.
 * @return A TimedAutomaton representing the Kleene-Star of the given automaton.
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> star(
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &&given) {
    return emptyOr(plus(std::move(given)));
}

/*!
 * @brief Compute the time restriction of the given timed automaton.
 *
 * @param[in] given The given timed automaton.
 * @param[in] guard The timing constraint that restricts the time.
 * @return A TimedAutomaton with the new timing constraint on the total elapsed time.
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> timeRestriction(
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &&given, TimingConstraint guard){
    given.clockVariableSize += 1; // Add a new clock variable for the time restriction

    // Add a new final state that has no outgoing transitions and add transitions to this state from all transitions to the original final states.
    auto newFinalState = std::make_shared<AutomatonState<StringConstraint, NumberConstraint, TimingConstraint, Update> >(true);
    given.states.push_back(newFinalState);
    guard = adjustDimension(guard, given.clockVariableSize); // Adjust the guard to include the new clock variable

    // For each transition, we update the dimensions of the guard
    for (const auto &sourceState: given.states) {
        for (auto &[label, transitions]: sourceState->next) {
            std::vector<AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update> > newTransitions;
            for (auto it = transitions.begin(); it != transitions.end();) {
                // Update the guard to include the new clock variable
                it->guard = adjustDimension(it->guard, given.clockVariableSize);

                // If the target state is a final state, we add a transition to the new final state
                if (it->target.lock()->isMatch) {
                    newTransitions.push_back({
                        it->stringConstraints,
                        it->numConstraints,
                        it->update,
                        it->resetVars,
                        guard && it->guard, // Combine the original guard with the new time restriction
                        newFinalState
                    });
                    // If the target state has no successor, we remove it
                    if (it->target.lock()->next.empty()) {
                        it = transitions.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    ++it; // If not a final state, just move to the next transition
                }
            }
            // Add the new transitions to the source state
            transitions.reserve(transitions.size() + newTransitions.size());
            std::move(newTransitions.begin(), newTransitions.end(), std::back_inserter(transitions));
        }
    }

    // Remove the original final states with no outgoing transitions
    for (auto it = given.states.begin(); it != given.states.end();) {
        if ((*it)->isMatch && (*it)->next.empty() && *it != newFinalState) {
            it = given.states.erase(it);
        } else {
            ++it;
        }
    }

    // Make the original final states non-final
    for (auto &state: given.states) {
        if (state != newFinalState && state->isMatch) {
            state->isMatch = false;
        }
    }

    return given;
}
