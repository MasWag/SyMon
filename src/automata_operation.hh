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
    for (const auto &source: right.states) {
        for (auto &[label, transitions]: source->next) {
            std::vector<AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update> >
                    newTransitions;
            for (auto it = transitions.begin(); it != transitions.end();) {
                if (it->target.lock()->isMatch) {
                    for (auto &ri: right.initialStates) {
                        // Create a new transition to the initial state of the right automaton
                        newTransitions.push_back({
                            it->stringConstraints, it->numConstraints, it->update,
                            it->resetVars, it->guard, ri
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
 * @brief Compute the Kleene-Plus of the given timed automaton.
 *
 * @param[in] given The given timed automaton.
 * @return A TimedAutomaton representing the concatenation of two automata.
 */
template<typename StringConstraint, typename NumberConstraint, typename TimingConstraint, typename Update>
TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> plus(
    TimedAutomaton<StringConstraint, NumberConstraint, TimingConstraint, Update> &&given) {
    // For each transition to the final state, we duplicate it and make a transition to the initial state
    
    for (const auto &sourceState : given.states) {
        for (auto &[label, transitions] : sourceState->next) {
            std::vector<AutomatonTransition<StringConstraint, NumberConstraint, TimingConstraint, Update>> newTransitions;
            
            for (const auto &transition : transitions) {
                if (transition.target.lock()->isMatch) {
                    // If this transition leads to a final state, duplicate it to also lead to all initial states
                    for (auto &initialState : given.initialStates) {
                        newTransitions.push_back({
                            transition.stringConstraints,
                            transition.numConstraints,
                            transition.update,
                            transition.resetVars,
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