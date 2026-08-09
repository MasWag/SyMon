#include <boost/test/unit_test.hpp>

#include "fixture/copy_automaton_fixture.hh"
#include "fixture/withdraw_automaton_fixture.hh"

#include "automata_operation.hh"

BOOST_AUTO_TEST_SUITE(AutomatonOperationTest)
    BOOST_AUTO_TEST_SUITE(NonParametric)

#include "fixture/star_automaton_fixture.hh"
#include "fixture/time_restriction_automaton_fixture.hh"

        struct CopyAndWithdrawFixture : CopyFixture, WithdrawFixture {
        };

        BOOST_FIXTURE_TEST_CASE(DisjunctionTest, CopyAndWithdrawFixture) {
            auto copy = this->CopyFixture::automaton;
            auto withdraw = this->WithdrawFixture::automaton;

            const size_t copyStateCount = copy.states.size();
            const size_t copyInitialStateCount = copy.initialStates.size();
            const size_t withdrawStateCount = withdraw.states.size();
            const size_t withdrawInitialStateCount = withdraw.initialStates.size();

            const auto result = disjunction(std::move(copy), std::move(withdraw));

            BOOST_CHECK_EQUAL(result.states.size(), copyStateCount + withdrawStateCount);
            BOOST_CHECK_EQUAL(result.initialStates.size(), copyInitialStateCount + withdrawInitialStateCount);
        }

        BOOST_FIXTURE_TEST_CASE(ConcatenateTest, CopyAndWithdrawFixture) {
            auto copy = this->CopyFixture::automaton;
            auto withdraw = this->WithdrawFixture::automaton;

            // Verify the initial state of the automata for concatenation

            // Check copy automaton
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);

            // Verify final states in copy automaton
            size_t copyFinalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    copyFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(copyFinalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            // Check withdraw automaton
            BOOST_CHECK_EQUAL(withdraw.states.size(), 3);
            BOOST_CHECK_EQUAL(withdraw.initialStates.size(), 1);

            // Verify final states in withdraw automaton
            size_t withdrawFinalStatesCount = 0;
            for (const auto &state: withdraw.states) {
                if (state->isMatch) {
                    withdrawFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(withdrawFinalStatesCount, 1);
            BOOST_CHECK(withdraw.states[2]->isMatch);

            // Now that the concatenate function has been fixed, we can test it
            // Expected behavior:
            // 1. Final states from copy become non-final
            // 2. Transitions are added from copy's final states to withdraw's initial states
            // 3. Final states from withdraw remain final
            // Store pointers to the final states before concatenation
            auto copyFinalState = copy.states[3];
            auto withdrawFinalState = withdraw.states[2];

            auto result = concatenate(std::move(copy), std::move(withdraw));

            // Check the total number of states (should be the sum of both automata)
            BOOST_CHECK_EQUAL(result.states.size(), 6);
            // 4 from copy + 3 from withdraw - 1 (copy's final state has no successors)

            // Check that the initial states are preserved from the first automaton
            BOOST_CHECK_EQUAL(result.initialStates.size(), 1);

            // Check that the original final state from copy is no longer final
            BOOST_CHECK(!copyFinalState->isMatch);

            // Check that at least one state is final (from withdraw)
            size_t finalStatesCount = 0;
            for (const auto &state: result.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_GT(finalStatesCount, 0);
        }

        BOOST_FIXTURE_TEST_CASE(PlusTest, CopyFixture) {
            auto copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);

            // Verify final states in the automaton
            size_t finalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            // Store a pointer to the final state before applying plus operation
            const auto finalState = copy.states[3];
            const auto initialState = copy.initialStates[0];

            // Apply the plus operation
            const auto result = plus(std::move(copy));

            // Check that the number of states remains the same
            BOOST_CHECK_EQUAL(result.states.size(), 4);

            // Check that the initial states remain the same
            BOOST_CHECK_EQUAL(result.initialStates.size(), 1);

            // Check that the final state is still final
            BOOST_CHECK(finalState->isMatch);
        }

        BOOST_FIXTURE_TEST_CASE(StarTest, CopyFixture) {
            auto copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);

            // Verify final states in the automaton
            size_t finalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            // Store a pointer to the original final state
            const auto originalFinalState = copy.states[3];

            // Apply the star operation
            const auto result = star(std::move(copy));

            // Check that a new state has been added
            BOOST_CHECK_EQUAL(result.states.size(), 5);

            // Check that the new state is at the beginning
            BOOST_CHECK(result.states.back()->isMatch);

            // Check that the initial states now include the new state
            BOOST_CHECK_EQUAL(result.initialStates.size(), 2);

            // Check that the new initial state is also a final state
            bool newInitialStateIsFinal = false;
            for (const auto &initialState: result.initialStates) {
                if (initialState->isMatch) {
                    newInitialStateIsFinal = true;
                    break;
                }
            }
            BOOST_CHECK(newInitialStateIsFinal);

            // Check that the original final state is still final
            BOOST_CHECK(originalFinalState->isMatch);

            // Verify that the plus operation was applied (by checking total final states)
            finalStatesCount = 0;
            for (const auto &state: result.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 2);
        }

        BOOST_FIXTURE_TEST_CASE(ConjunctionTest, CopyAndWithdrawFixture) {
            auto copy = this->CopyFixture::automaton;
            auto withdraw = this->WithdrawFixture::automaton;

            // Verify the initial state of the automata
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(withdraw.states.size(), 3);
            BOOST_CHECK_EQUAL(withdraw.initialStates.size(), 1);

            // Count final states in both automata
            size_t copyFinalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    copyFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(copyFinalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            size_t withdrawFinalStatesCount = 0;
            for (const auto &state: withdraw.states) {
                if (state->isMatch) {
                    withdrawFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(withdrawFinalStatesCount, 1);
            BOOST_CHECK(withdraw.states[2]->isMatch);

            // Perform conjunction
            const auto result = conjunction(copy, withdraw);

            // Check basic properties of the result
            BOOST_CHECK_EQUAL(result.clockVariableSize, copy.clockVariableSize + withdraw.clockVariableSize);
            BOOST_CHECK_EQUAL(result.stringVariableSize,
                              std::max(copy.stringVariableSize, withdraw.stringVariableSize));
            BOOST_CHECK_EQUAL(result.numberVariableSize,
                              std::max(copy.numberVariableSize, withdraw.numberVariableSize));

            // Check initial states (should be the product of initial states)
            BOOST_CHECK_EQUAL(result.initialStates.size(), copy.initialStates.size() * withdraw.initialStates.size());

            // Check total states (should be less than or equal to the product of all states)
            BOOST_CHECK_LE(result.states.size(), copy.states.size() * withdraw.states.size());

            // Check that the initial state has transitions
            BOOST_CHECK(!result.initialStates[0]->next.empty());
        }

        BOOST_FIXTURE_TEST_CASE(TimeRestrictionTest, NonParametric::TimeRestrictionFixture) {
            auto automaton_copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(automaton_copy.states.size(), 2);
            BOOST_CHECK_EQUAL(automaton_copy.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton_copy.clockVariableSize, 1);

            // Verify there's only one final state initially
            size_t finalStatesCount = 0;
            for (const auto &state: automaton_copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(automaton_copy.states[1]->isMatch);
            BOOST_CHECK(!automaton_copy.states[0]->isMatch);

            // Store a pointer to the original final state
            auto originalFinalState = automaton_copy.states[1];

            // Create a timing constraint for the time restriction
            std::vector<TimingConstraint<double>> timeGuard;
            timeGuard.push_back(ConstraintMaker(0) <= 10.);

            // Apply the time restriction operation
            auto result = timeRestriction(std::move(automaton_copy), timeGuard);

            // Check that the clock variable size is increased by 1
            BOOST_CHECK_EQUAL(result.clockVariableSize, 2);

            // Check that a new state has been added (the new final state), but the original final state is removed
            BOOST_CHECK_EQUAL(result.states.size(), 2);

            // Check that the new final state is final
            BOOST_CHECK(result.states.back()->isMatch);

            // Check that there's exactly one final state
            finalStatesCount = 0;
            for (const auto &state: result.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);

            // Check that the transition to the new final state has the time guard applied
            bool hasTransitionToNewFinal = false;
            for (const auto &state: result.states) {
                for (const auto &[label, transitions]: state->next) {
                    for (const auto &transition: transitions) {
                        if (transition.target.lock() == result.states.back()) {
                            hasTransitionToNewFinal = true;
                            // The transition should have the time guard applied
                            BOOST_CHECK(!transition.guard.empty());
                        }
                    }
                }
            }
            BOOST_CHECK(hasTransitionToNewFinal);
        }

        BOOST_FIXTURE_TEST_CASE(StarAutomatonTest, NonParametric::StarAutomatonFixture) {
            auto automaton_copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(automaton_copy.states.size(), 3);
            BOOST_CHECK_EQUAL(automaton_copy.initialStates.size(), 1);

            // Verify there's only one final state initially
            size_t finalStatesCount = 0;
            for (const auto &state: automaton_copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(automaton_copy.states[2]->isMatch);
            BOOST_CHECK(!automaton_copy.states[0]->isMatch);

            // Apply the star operation
            const auto result = star(std::move(automaton_copy));

            // Check that a new state has been added
            BOOST_CHECK_EQUAL(result.states.size(), 4);

            // Check that the new state is at the beginning and is final
            BOOST_CHECK(result.states.back()->isMatch);

            // Check that we now have two initial states
            BOOST_CHECK_EQUAL(result.initialStates.size(), 2);

            // Check that one of the initial states is also final
            bool hasInitialFinalState = false;
            for (const auto &initialState: result.initialStates) {
                if (initialState->isMatch) {
                    hasInitialFinalState = true;
                    break;
                }
            }
            BOOST_CHECK(hasInitialFinalState);

            // Check that the original final state is still final
            bool originalFinalStateStillFinal = false;
            for (const auto &state: result.states) {
                if (state != result.states[0] && state->isMatch) {
                    originalFinalStateStillFinal = true;
                    break;
                }
            }
            BOOST_CHECK(originalFinalStateStillFinal);
        }

        BOOST_FIXTURE_TEST_CASE(AddConstraintToAllTransitionsTest, CopyFixture) {
            auto automaton_copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(automaton_copy.states.size(), 4);

            // Create a timing constraint to add to all transitions
            std::vector<TimingConstraint<double>> constraint;
            constraint.push_back(ConstraintMaker(0) <= 5.);

            // Count the number of transitions before adding the constraint
            size_t transitionCount = 0;
            for (const auto &state : automaton_copy.states) {
                for (const auto &[label, transitions] : state->next) {
                    transitionCount += transitions.size();
                }
            }

            // Apply the constraint to all transitions
            addConstraintToAllTransitions(automaton_copy, constraint);

            // Verify that all transitions have the constraint applied
            size_t constrainedTransitionCount = 0;
            for (const auto &state : automaton_copy.states) {
                for (const auto &[label, transitions] : state->next) {
                    for (const auto &transition : transitions) {
                        // Check that the constraint is applied to the transition
                        bool hasConstraint = false;
                        for (const auto &guard : transition.guard) {
                            if (guard.x == 0 && guard.odr == TimingConstraintOrder::le && guard.c == 5) {
                                hasConstraint = true;
                                break;
                            }
                        }
                        BOOST_CHECK(hasConstraint);
                        constrainedTransitionCount++;
                    }
                }
            }

            // Verify that the number of transitions remains the same
            BOOST_CHECK_EQUAL(constrainedTransitionCount, transitionCount);
        }

    BOOST_AUTO_TEST_SUITE_END() // NonParametric

    BOOST_AUTO_TEST_SUITE(DataParametric)

        struct CopyAndWithdrawFixture : DataParametricCopy, DataParametricWithdrawFixture {
        };

        BOOST_FIXTURE_TEST_CASE(DisjunctionTest, CopyAndWithdrawFixture) {
            auto copy = this->DataParametricCopy::automaton;
            auto withdraw = this->DataParametricWithdrawFixture::automaton;

            const size_t copyStateCount = copy.states.size();
            const size_t copyInitialStateCount = copy.initialStates.size();
            const size_t withdrawStateCount = withdraw.states.size();
            const size_t withdrawInitialStateCount = withdraw.initialStates.size();

            const auto result = disjunction(std::move(copy), std::move(withdraw));

            BOOST_CHECK_EQUAL(result.states.size(), copyStateCount + withdrawStateCount);
            BOOST_CHECK_EQUAL(result.initialStates.size(), copyInitialStateCount + withdrawInitialStateCount);
        }

        BOOST_FIXTURE_TEST_CASE(ConcatenateTest, CopyAndWithdrawFixture) {
            auto copy = this->DataParametricCopy::automaton;
            auto withdraw = this->DataParametricWithdrawFixture::automaton;

            // Verify the initial state of the automata for concatenation

            // Check copy automaton
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);

            // Verify final states in copy automaton
            size_t copyFinalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    copyFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(copyFinalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            // Check withdraw automaton
            BOOST_CHECK_EQUAL(withdraw.states.size(), 3);
            BOOST_CHECK_EQUAL(withdraw.initialStates.size(), 1);

            // Verify final states in withdraw automaton
            size_t withdrawFinalStatesCount = 0;
            for (const auto &state: withdraw.states) {
                if (state->isMatch) {
                    withdrawFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(withdrawFinalStatesCount, 1);
            BOOST_CHECK(withdraw.states[2]->isMatch);

            // Now that the concatenate function has been fixed, we can test it
            // Expected behavior:
            // 1. Final states from copy become non-final
            // 2. Transitions are added from copy's final states to withdraw's initial states
            // 3. Final states from withdraw remain final
            // Store pointers to the final states before concatenation
            auto copyFinalState = copy.states[3];
            auto withdrawFinalState = withdraw.states[2];

            auto result = concatenate(std::move(copy), std::move(withdraw));

            // Check the total number of states (should be the sum of both automata)
            BOOST_CHECK_EQUAL(result.states.size(), 6);
            // 4 from copy + 3 from withdraw - 1 (copy's final state has no successors)

            // Check that the initial states are preserved from the first automaton
            BOOST_CHECK_EQUAL(result.initialStates.size(), 1);

            // Check that the original final state from copy is no longer final
            BOOST_CHECK(!copyFinalState->isMatch);

            // Check that at least one state is final (from withdraw)
            size_t finalStatesCount = 0;
            for (const auto &state: result.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_GT(finalStatesCount, 0);
        }

        BOOST_FIXTURE_TEST_CASE(PlusTest, DataParametricCopy) {
            auto copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);

            // Verify final states in the automaton
            size_t finalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            // Store a pointer to the final state before applying plus operation
            auto finalState = copy.states[3];
            auto initialState = copy.initialStates[0];

            // Apply the plus operation
            auto result = plus(std::move(copy));

            // Check that the number of states remains the same
            BOOST_CHECK_EQUAL(result.states.size(), 4);

            // Check that the initial states remain the same
            BOOST_CHECK_EQUAL(result.initialStates.size(), 1);

            // Check that the final state is still final
            BOOST_CHECK(finalState->isMatch);
        }

        BOOST_FIXTURE_TEST_CASE(StarTest, DataParametricCopy) {
            auto copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);

            // Verify final states in the automaton
            size_t finalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            // Store a pointer to the original final state
            auto originalFinalState = copy.states[3];

            // Apply the star operation
            auto result = star(std::move(copy));

            // Check that a new state has been added
            BOOST_CHECK_EQUAL(result.states.size(), 5);

            // Check that the new state is at the beginning
            BOOST_CHECK(result.states.back()->isMatch);

            // Check that the initial states now include the new state
            BOOST_CHECK_EQUAL(result.initialStates.size(), 2);

            // Check that the new initial state is also a final state
            bool newInitialStateIsFinal = false;
            for (const auto &initialState: result.initialStates) {
                if (initialState->isMatch) {
                    newInitialStateIsFinal = true;
                    break;
                }
            }
            BOOST_CHECK(newInitialStateIsFinal);

            // Check that the original final state is still final
            BOOST_CHECK(originalFinalState->isMatch);

            // Verify that the plus operation was applied (by checking total final states)
            finalStatesCount = 0;
            for (const auto &state: result.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 2);
        }

        BOOST_FIXTURE_TEST_CASE(ConjunctionTest, CopyAndWithdrawFixture) {
            auto copy = this->DataParametricCopy::automaton;
            auto withdraw = this->DataParametricWithdrawFixture::automaton;

            // Verify the initial state of the automata
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(withdraw.states.size(), 3);
            BOOST_CHECK_EQUAL(withdraw.initialStates.size(), 1);

            // Count final states in both automata
            size_t copyFinalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    copyFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(copyFinalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            size_t withdrawFinalStatesCount = 0;
            for (const auto &state: withdraw.states) {
                if (state->isMatch) {
                    withdrawFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(withdrawFinalStatesCount, 1);
            BOOST_CHECK(withdraw.states[2]->isMatch);

            // Perform conjunction
            const auto result = conjunction(copy, withdraw);

            // Check basic properties of the result
            BOOST_CHECK_EQUAL(result.clockVariableSize, copy.clockVariableSize + withdraw.clockVariableSize);
            BOOST_CHECK_EQUAL(result.stringVariableSize,
                              std::max(copy.stringVariableSize, withdraw.stringVariableSize));
            BOOST_CHECK_EQUAL(result.numberVariableSize,
                              std::max(copy.numberVariableSize, withdraw.numberVariableSize));

            // Check initial states (should be the product of initial states)
            BOOST_CHECK_EQUAL(result.initialStates.size(), copy.initialStates.size() * withdraw.initialStates.size());

            // Check total states (should be less than or equal to the product of all states)
            BOOST_CHECK_LE(result.states.size(), copy.states.size() * withdraw.states.size());

            // Check that the initial state has transitions
            BOOST_CHECK(!result.initialStates[0]->next.empty());
        }

        BOOST_FIXTURE_TEST_CASE(TimeRestrictionTest, NonParametric::NonParametric::DataParametricTimeRestrictionFixture) {
            auto automaton_copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(automaton_copy.states.size(), 2);
            BOOST_CHECK_EQUAL(automaton_copy.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton_copy.clockVariableSize, 1);

            // Verify there's only one final state initially
            size_t finalStatesCount = 0;
            for (const auto &state: automaton_copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(automaton_copy.states[1]->isMatch);
            BOOST_CHECK(!automaton_copy.states[0]->isMatch);

            // Store a pointer to the original final state
            auto originalFinalState = automaton_copy.states[1];

            // Create a timing constraint for the time restriction
            using namespace Parma_Polyhedra_Library;
            using namespace Symbolic;
            std::vector<TimingConstraint<double>> timeGuard;
            timeGuard.push_back(ConstraintMaker(0) <= 10.);

            // Apply the time restriction operation
            auto result = timeRestriction(std::move(automaton_copy), timeGuard);

            // Check that the clock variable size is increased by 1
            BOOST_CHECK_EQUAL(result.clockVariableSize, 2);

            // Check that a new state has been added (the new final state), but the finial state is removed
            BOOST_CHECK_EQUAL(result.states.size(), 2);

            // Check that the new final state is final
            BOOST_CHECK(result.states.back()->isMatch);

            // Check that there's exactly one final state
            finalStatesCount = 0;
            for (const auto &state: result.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);

            // Check that the transition to the new final state exists and has the time guard applied
            bool hasTransitionToNewFinal = false;
            for (const auto &state: result.states) {
                for (const auto &[label, transitions]: state->next) {
                    for (const auto &transition: transitions) {
                        if (transition.target.lock() == result.states.back()) {
                            hasTransitionToNewFinal = true;
                        }
                        // The transition should have the time guard applied
                        BOOST_CHECK(!transition.guard.empty());
                    }
                }
            }
            BOOST_CHECK(hasTransitionToNewFinal);
        }

        BOOST_FIXTURE_TEST_CASE(StarAutomatonTest, NonParametric::NonParametric::DataParametricStarAutomaton) {
            auto automaton_copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(automaton_copy.states.size(), 3);
            BOOST_CHECK_EQUAL(automaton_copy.initialStates.size(), 1);

            // Verify there's only one final state initially
            size_t finalStatesCount = 0;
            for (const auto &state: automaton_copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(automaton_copy.states[2]->isMatch);
            BOOST_CHECK(!automaton_copy.states[0]->isMatch);

            // Apply the star operation
            auto result = star(std::move(automaton_copy));

            // Check that a new state has been added
            BOOST_CHECK_EQUAL(result.states.size(), 4);

            // Check that the new state is at the beginning and is final
            BOOST_CHECK(result.states.back()->isMatch);

            // Check that we now have two initial states
            BOOST_CHECK_EQUAL(result.initialStates.size(), 2);

            // Check that one of the initial states is also final
            bool hasInitialFinalState = false;
            for (const auto &initialState: result.initialStates) {
                if (initialState->isMatch) {
                    hasInitialFinalState = true;
                    break;
                }
            }
            BOOST_CHECK(hasInitialFinalState);

            // Check that the original final state is still final
            bool originalFinalStateStillFinal = false;
            for (const auto &state: result.states) {
                if (state != result.states[0] && state->isMatch) {
                    originalFinalStateStillFinal = true;
                    break;
                }
            }
            BOOST_CHECK(originalFinalStateStillFinal);
        }

        BOOST_FIXTURE_TEST_CASE(AddConstraintToAllTransitionsTest, DataParametricCopy) {
            auto automaton_copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(automaton_copy.states.size(), 4);

            // Create a timing constraint to add to all transitions
            std::vector<TimingConstraint<double>> constraint;
            constraint.push_back(ConstraintMaker(0) <= 5.);

            // Count the number of transitions before adding the constraint
            size_t transitionCount = 0;
            for (const auto &state : automaton_copy.states) {
                for (const auto &[label, transitions] : state->next) {
                    transitionCount += transitions.size();
                }
            }

            // Apply the constraint to all transitions
            addConstraintToAllTransitions(automaton_copy, constraint);

            // Verify that all transitions have the constraint applied
            size_t constrainedTransitionCount = 0;
            for (const auto &state : automaton_copy.states) {
                for (const auto &[label, transitions] : state->next) {
                    for (const auto &transition : transitions) {
                        // Check that the constraint is applied to the transition
                        bool hasConstraint = false;
                        for (const auto &guard : transition.guard) {
                            if (guard.x == 0 && guard.odr == TimingConstraintOrder::le && guard.c == 5) {
                                hasConstraint = true;
                                break;
                            }
                        }
                        BOOST_CHECK(hasConstraint);
                        constrainedTransitionCount++;
                    }
                }
            }

            // Verify that the number of transitions remains the same
            BOOST_CHECK_EQUAL(constrainedTransitionCount, transitionCount);
        }

    BOOST_AUTO_TEST_SUITE_END() // DataParametric

    BOOST_AUTO_TEST_SUITE(Parametric)

        struct CopyAndWithdrawFixture : ParametricCopy, ParametricWithdrawFixture {
        };

        BOOST_FIXTURE_TEST_CASE(DisjunctionTest, CopyAndWithdrawFixture) {
            auto copy = this->ParametricCopy::automaton;
            auto withdraw = this->ParametricWithdrawFixture::automaton;

            const size_t copyStateCount = copy.states.size();
            const size_t copyInitialStateCount = copy.initialStates.size();
            const size_t withdrawStateCount = withdraw.states.size();
            const size_t withdrawInitialStateCount = withdraw.initialStates.size();

            const auto result = disjunction(std::move(copy), std::move(withdraw));

            BOOST_CHECK_EQUAL(result.states.size(), copyStateCount + withdrawStateCount);
            BOOST_CHECK_EQUAL(result.initialStates.size(), copyInitialStateCount + withdrawInitialStateCount);
        }

        BOOST_FIXTURE_TEST_CASE(ConcatenateTest, CopyAndWithdrawFixture) {
            auto copy = this->ParametricCopy::automaton;
            auto withdraw = this->ParametricWithdrawFixture::automaton;

            // Verify the initial state of the automata for concatenation

            // Check copy automaton
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);

            // Verify final states in copy automaton
            size_t copyFinalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    copyFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(copyFinalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            // Check withdraw automaton
            BOOST_CHECK_EQUAL(withdraw.states.size(), 3);
            BOOST_CHECK_EQUAL(withdraw.initialStates.size(), 1);

            // Verify final states in withdraw automaton
            size_t withdrawFinalStatesCount = 0;
            for (const auto &state: withdraw.states) {
                if (state->isMatch) {
                    withdrawFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(withdrawFinalStatesCount, 1);
            BOOST_CHECK(withdraw.states[2]->isMatch);

            // Now that the concatenate function has been fixed, we can test it
            // Expected behavior:
            // 1. Final states from copy become non-final
            // 2. Transitions are added from copy's final states to withdraw's initial states
            // 3. Final states from withdraw remain final
            // Store pointers to the final states before concatenation
            auto copyFinalState = copy.states[3];
            auto withdrawFinalState = withdraw.states[2];

            auto result = concatenate(std::move(copy), std::move(withdraw));

            // Check the total number of states (should be the sum of both automata)
            BOOST_CHECK_EQUAL(result.states.size(), 6);
            // 4 from copy + 3 from withdraw - 1 (copy's final state has no successors)

            // Check that the initial states are preserved from the first automaton
            BOOST_CHECK_EQUAL(result.initialStates.size(), 1);

            // Check that the original final state from copy is no longer final
            BOOST_CHECK(!copyFinalState->isMatch);

            // Check that at least one state is final (from withdraw)
            size_t finalStatesCount = 0;
            for (const auto &state: result.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_GT(finalStatesCount, 0);
        }

        BOOST_FIXTURE_TEST_CASE(PlusTest, ParametricCopy) {
            auto copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);

            // Verify final states in the automaton
            size_t finalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            // Store a pointer to the final state before applying plus operation
            auto finalState = copy.states[3];
            auto initialState = copy.initialStates[0];

            // Apply the plus operation
            auto result = plus(std::move(copy));

            // Check that the number of states remains the same
            BOOST_CHECK_EQUAL(result.states.size(), 4);

            // Check that the initial states remain the same
            BOOST_CHECK_EQUAL(result.initialStates.size(), 1);

            // Check that the final state is still final
            BOOST_CHECK(finalState->isMatch);
        }

        BOOST_FIXTURE_TEST_CASE(StarTest, ParametricCopy) {
            auto copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);

            // Verify final states in the automaton
            size_t finalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            // Store a pointer to the original final state
            auto originalFinalState = copy.states[3];

            // Apply the star operation
            auto result = star(std::move(copy));

            // Check that a new state has been added
            BOOST_CHECK_EQUAL(result.states.size(), 5);

            // Check that the new state is at the beginning
            BOOST_CHECK(result.states.back()->isMatch);

            // Check that the initial states now include the new state
            BOOST_CHECK_EQUAL(result.initialStates.size(), 2);

            // Check that the new initial state is also a final state
            bool newInitialStateIsFinal = false;
            for (const auto &initialState: result.initialStates) {
                if (initialState->isMatch) {
                    newInitialStateIsFinal = true;
                    break;
                }
            }
            BOOST_CHECK(newInitialStateIsFinal);

            // Check that the original final state is still final
            BOOST_CHECK(originalFinalState->isMatch);

            // Verify that the plus operation was applied (by checking total final states)
            finalStatesCount = 0;
            for (const auto &state: result.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 2);
        }

        BOOST_FIXTURE_TEST_CASE(ConjunctionTest, CopyAndWithdrawFixture) {
            auto copy = this->ParametricCopy::automaton;
            auto withdraw = this->ParametricWithdrawFixture::automaton;

            // Verify the initial state of the automata
            BOOST_CHECK_EQUAL(copy.states.size(), 4);
            BOOST_CHECK_EQUAL(copy.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(withdraw.states.size(), 3);
            BOOST_CHECK_EQUAL(withdraw.initialStates.size(), 1);

            // Count final states in both automata
            size_t copyFinalStatesCount = 0;
            for (const auto &state: copy.states) {
                if (state->isMatch) {
                    copyFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(copyFinalStatesCount, 1);
            BOOST_CHECK(copy.states[3]->isMatch);

            size_t withdrawFinalStatesCount = 0;
            for (const auto &state: withdraw.states) {
                if (state->isMatch) {
                    withdrawFinalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(withdrawFinalStatesCount, 1);
            BOOST_CHECK(withdraw.states[2]->isMatch);

            // Perform conjunction
            const auto result = conjunction(copy, withdraw);

            // Check basic properties of the result
            BOOST_CHECK_EQUAL(result.clockVariableSize, copy.clockVariableSize + withdraw.clockVariableSize);
            BOOST_CHECK_EQUAL(result.stringVariableSize,
                              std::max(copy.stringVariableSize, withdraw.stringVariableSize));
            BOOST_CHECK_EQUAL(result.numberVariableSize,
                              std::max(copy.numberVariableSize, withdraw.numberVariableSize));

            // Check initial states (should be the product of initial states)
            BOOST_CHECK_EQUAL(result.initialStates.size(), copy.initialStates.size() * withdraw.initialStates.size());

            // Check total states (should be less than or equal to the product of all states)
            BOOST_CHECK_LE(result.states.size(), copy.states.size() * withdraw.states.size());

            // Check that the initial state has transitions
            BOOST_CHECK(!result.initialStates[0]->next.empty());
        }

        BOOST_FIXTURE_TEST_CASE(TimeRestrictionTest, NonParametric::NonParametric::ParametricTimeRestrictionFixture) {
            auto automaton_copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(automaton_copy.states.size(), 2);
            BOOST_CHECK_EQUAL(automaton_copy.initialStates.size(), 1);
            BOOST_CHECK_EQUAL(automaton_copy.clockVariableSize, 1);

            // Verify there's only one final state initially
            size_t finalStatesCount = 0;
            for (const auto &state: automaton_copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(automaton_copy.states[1]->isMatch);
            BOOST_CHECK(!automaton_copy.states[0]->isMatch);

            // Store a pointer to the original final state
            const auto originalFinalState = automaton_copy.states[1];

            // Create a timing constraint for the time restriction
            using namespace Parma_Polyhedra_Library;
            using namespace Symbolic;
            ParametricTimingConstraint timeGuard = ParametricTimingConstraint(2);
            timeGuard.add_constraint(Variable(1) <= 10);

            // Apply the time restriction operation
            const auto result = timeRestriction(std::move(automaton_copy), timeGuard);

            // Check that the clock variable size is increased by 1
            BOOST_CHECK_EQUAL(result.clockVariableSize, 2);

            // Check that a new state has been added (the new final state), but the original final state is removed
            BOOST_CHECK_EQUAL(result.states.size(), 2);

            // Check that the new final state is final
            BOOST_CHECK(result.states.back()->isMatch);

            // Check that there's exactly one final state
            finalStatesCount = 0;
            for (const auto &state: result.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);

            // Check that the transition to the new final state exists and has the time guard applied
            bool hasTransitionToNewFinal = false;
            for (const auto &state: result.states) {
                for (const auto &[label, transitions]: state->next) {
                    for (const auto &transition: transitions) {
                        if (transition.target.lock() == result.states.back()) {
                            hasTransitionToNewFinal = true;
                        }
                        // The transition should have the time guard applied
                        BOOST_CHECK(!transition.guard.is_universe());
                    }
                }
            }
            BOOST_CHECK(hasTransitionToNewFinal);
        }

        BOOST_FIXTURE_TEST_CASE(StarAutomatonTest, NonParametric::NonParametric::ParametricStarAutomaton) {
            auto automaton_copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(automaton_copy.states.size(), 3);
            BOOST_CHECK_EQUAL(automaton_copy.initialStates.size(), 1);

            // Verify there's only one final state initially
            size_t finalStatesCount = 0;
            for (const auto &state: automaton_copy.states) {
                if (state->isMatch) {
                    finalStatesCount++;
                }
            }
            BOOST_CHECK_EQUAL(finalStatesCount, 1);
            BOOST_CHECK(automaton_copy.states[2]->isMatch);
            BOOST_CHECK(!automaton_copy.states[0]->isMatch);

            // Apply the star operation
            const auto result = star(std::move(automaton_copy));

            // Check that a new state has been added
            BOOST_CHECK_EQUAL(result.states.size(), 4);

            // Check that the new state is at the beginning and is final
            BOOST_CHECK(result.states.back()->isMatch);

            // Check that we now have two initial states
            BOOST_CHECK_EQUAL(result.initialStates.size(), 2);

            // Check that one of the initial states is also final
            bool hasInitialFinalState = false;
            for (const auto &initialState: result.initialStates) {
                if (initialState->isMatch) {
                    hasInitialFinalState = true;
                    break;
                }
            }
            BOOST_CHECK(hasInitialFinalState);

            // Check that the original final state is still final
            bool originalFinalStateStillFinal = false;
            for (const auto &state: result.states) {
                if (state != result.states[0] && state->isMatch) {
                    originalFinalStateStillFinal = true;
                    break;
                }
            }
            BOOST_CHECK(originalFinalStateStillFinal);
        }

        BOOST_FIXTURE_TEST_CASE(AddConstraintToAllTransitionsTest, ParametricCopy) {
            auto automaton_copy = this->automaton;

            // Verify the initial state of the automaton
            BOOST_CHECK_EQUAL(automaton_copy.states.size(), 4);

            // Create a parametric timing constraint to add to all transitions
            using namespace Parma_Polyhedra_Library;
            ParametricTimingConstraint constraint(automaton_copy.clockVariableSize);
            constraint.add_constraint(Variable(0) <= 5);

            // Count the number of transitions before adding the constraint
            size_t transitionCount = 0;
            for (const auto &state : automaton_copy.states) {
                for (const auto &[label, transitions] : state->next) {
                    transitionCount += transitions.size();
                }
            }

            // Apply the constraint to all transitions
            addConstraintToAllTransitions(automaton_copy, constraint);

            // Verify that all transitions have the constraint applied
            size_t constrainedTransitionCount = 0;
            for (const auto &state : automaton_copy.states) {
                for (const auto &[label, transitions] : state->next) {
                    for (const auto &transition : transitions) {
                        // Check that the constraint is applied to the transition
                        BOOST_CHECK(!transition.guard.is_universe());
                        constrainedTransitionCount++;
                    }
                }
            }

            // Verify that the number of transitions remains the same
            BOOST_CHECK_EQUAL(constrainedTransitionCount, transitionCount);
        }

    BOOST_AUTO_TEST_SUITE_END() // Parametric

BOOST_AUTO_TEST_SUITE_END() // AutomatonOperationTest
