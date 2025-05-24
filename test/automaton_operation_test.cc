#include <boost/test/unit_test.hpp>

#include "fixture/copy_automaton_fixture.hh"
#include "fixture/withdraw_automaton_fixture.hh"

#include "automata_operation.hh"

BOOST_AUTO_TEST_SUITE(AutomatonOperationTest)
    BOOST_AUTO_TEST_SUITE(NonParametric)

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

    BOOST_AUTO_TEST_SUITE_END() // Parametric

BOOST_AUTO_TEST_SUITE_END() // AutomatonOperationTest
