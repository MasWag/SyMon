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
