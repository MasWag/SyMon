#include <boost/test/unit_test.hpp>

#include "fixture/copy_automaton_fixture.hh"
#include "fixture/withdraw_automaton_fixture.hh"
#include "fixture/star_automaton_fixture.hh"

BOOST_AUTO_TEST_SUITE(AutomatonDeepCopyTest)

    BOOST_AUTO_TEST_SUITE(NonParametric)

        BOOST_FIXTURE_TEST_CASE(BasicDeepCopyTest, CopyFixture) {
            // Create a deep copy of the automaton
            auto copy = this->automaton.deepCopy();

            // Check that the copy has the same number of states and initial states
            BOOST_CHECK_EQUAL(copy.states.size(), this->automaton.states.size());
            BOOST_CHECK_EQUAL(copy.initialStates.size(), this->automaton.initialStates.size());

            // Check that the copy is not the same object as the original
            BOOST_CHECK(&copy != &this->automaton);

            // Check that the states in the copy are not the same objects as the states in the original
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                BOOST_CHECK(copy.states[i].get() != this->automaton.states[i].get());
                BOOST_CHECK_EQUAL(copy.states[i]->isMatch, this->automaton.states[i]->isMatch);
            }

            // Check that the initial states in the copy point to the correct copied states
            for (size_t i = 0; i < this->automaton.initialStates.size(); ++i) {
                // Find the index of the initial state in the original automaton
                size_t originalIndex = 0;
                for (; originalIndex < this->automaton.states.size(); ++originalIndex) {
                    if (this->automaton.states[originalIndex] == this->automaton.initialStates[i]) {
                        break;
                    }
                }
                BOOST_CHECK_LT(originalIndex, this->automaton.states.size());

                // Check that the initial state in the copy points to the corresponding state
                BOOST_CHECK(copy.initialStates[i] == copy.states[originalIndex]);
            }

            // Check that the transitions in the copy point to the correct copied states
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                const auto& originalState = this->automaton.states[i];
                const auto& copiedState = copy.states[i];

                // Check that the transitions map has the same keys
                BOOST_CHECK_EQUAL(originalState->next.size(), copiedState->next.size());

                for (const auto& [action, originalTransitions] : originalState->next) {
                    // Check that the action exists in the copied state
                    BOOST_CHECK(copiedState->next.find(action) != copiedState->next.end());

                    const auto& copiedTransitions = copiedState->next.at(action);
                    BOOST_CHECK_EQUAL(originalTransitions.size(), copiedTransitions.size());

                    // Check each transition
                    for (size_t j = 0; j < originalTransitions.size(); ++j) {
                        const auto& originalTransition = originalTransitions[j];
                        const auto& copiedTransition = copiedTransitions[j];

                        // Check that the transition properties are the same
                        BOOST_CHECK_EQUAL(originalTransition.stringConstraints.size(), copiedTransition.stringConstraints.size());
                        BOOST_CHECK_EQUAL(originalTransition.numConstraints.size(), copiedTransition.numConstraints.size());
                        BOOST_CHECK_EQUAL(originalTransition.resetVars.size(), copiedTransition.resetVars.size());
                        BOOST_CHECK_EQUAL(originalTransition.guard.size(), copiedTransition.guard.size());

                        // Check that the target state is correctly mapped
                        auto originalTarget = originalTransition.target.lock();
                        auto copiedTarget = copiedTransition.target.lock();

                        BOOST_CHECK(originalTarget != copiedTarget);

                        // Find the index of the target state in the original automaton
                        size_t targetIndex = 0;
                        for (; targetIndex < this->automaton.states.size(); ++targetIndex) {
                            if (this->automaton.states[targetIndex] == originalTarget) {
                                break;
                            }
                        }
                        BOOST_CHECK_LT(targetIndex, this->automaton.states.size());

                        // Check that the target state in the copy points to the corresponding state
                        BOOST_CHECK(copiedTarget == copy.states[targetIndex]);
                    }
                }
            }

            // Check that the TimedAutomaton specific members are copied correctly
            BOOST_CHECK_EQUAL(copy.stringVariableSize, this->automaton.stringVariableSize);
            BOOST_CHECK_EQUAL(copy.numberVariableSize, this->automaton.numberVariableSize);
            BOOST_CHECK_EQUAL(copy.clockVariableSize, this->automaton.clockVariableSize);
        }

        BOOST_FIXTURE_TEST_CASE(ModifyAfterCopyTest, CopyFixture) {
            // Create a deep copy of the automaton
            auto copy = this->automaton.deepCopy();

            // Modify the original automaton
            this->automaton.states[0]->isMatch = true;
            this->automaton.clockVariableSize = 99;

            // Check that the copy is not affected by the changes to the original
            BOOST_CHECK(!copy.states[0]->isMatch);
            BOOST_CHECK_EQUAL(copy.clockVariableSize, 1);
        }

        BOOST_FIXTURE_TEST_CASE(ModifyCopyTest, CopyFixture) {
            // Create a deep copy of the automaton
            auto copy = this->automaton.deepCopy();

            // Modify the copy
            copy.states[0]->isMatch = true;
            copy.clockVariableSize = 99;

            // Add a new transition in the copy
            auto& transitions = copy.states[0]->next[0];
            auto newTransition = transitions[0]; // Clone an existing transition
            transitions.push_back(newTransition);

            // Check that the original is not affected by the changes to the copy
            BOOST_CHECK(!this->automaton.states[0]->isMatch);
            BOOST_CHECK_EQUAL(this->automaton.clockVariableSize, 1);
            BOOST_CHECK_EQUAL(this->automaton.states[0]->next[0].size(), 2); // Original should still have 2 transitions
        }

        BOOST_FIXTURE_TEST_CASE(WithdrawAutomatonTest, WithdrawFixture) {
            // Create a deep copy of the withdraw automaton
            auto copy = this->automaton.deepCopy();

            // Check that the copy has the same number of states and initial states
            BOOST_CHECK_EQUAL(copy.states.size(), this->automaton.states.size());
            BOOST_CHECK_EQUAL(copy.initialStates.size(), this->automaton.initialStates.size());

            // Check that the copy is not the same object as the original
            BOOST_CHECK(&copy != &this->automaton);

            // Check that the states in the copy are not the same objects as the states in the original
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                BOOST_CHECK(copy.states[i].get() != this->automaton.states[i].get());
                BOOST_CHECK_EQUAL(copy.states[i]->isMatch, this->automaton.states[i]->isMatch);
            }

            // Check that the transitions in the copy point to the correct copied states
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                const auto& originalState = this->automaton.states[i];
                const auto& copiedState = copy.states[i];

                // Check that the transitions map has the same keys
                BOOST_CHECK_EQUAL(originalState->next.size(), copiedState->next.size());

                for (const auto& [action, originalTransitions] : originalState->next) {
                    const auto& copiedTransitions = copiedState->next.at(action);
                    BOOST_CHECK_EQUAL(originalTransitions.size(), copiedTransitions.size());
                }
            }

            // Modify the original automaton
            this->automaton.states[1]->isMatch = true;
            this->automaton.stringVariableSize = 99;

            // Check that the copy is not affected by the changes to the original
            BOOST_CHECK(!copy.states[1]->isMatch);
            BOOST_CHECK_EQUAL(copy.stringVariableSize, 1);
        }

    BOOST_AUTO_TEST_SUITE_END() // NonParametric

    BOOST_AUTO_TEST_SUITE(DataParametric)

        BOOST_FIXTURE_TEST_CASE(BasicDeepCopyTest, DataParametricCopy) {
            // Create a deep copy of the automaton
            auto copy = this->automaton.deepCopy();

            // Check that the copy has the same number of states and initial states
            BOOST_CHECK_EQUAL(copy.states.size(), this->automaton.states.size());
            BOOST_CHECK_EQUAL(copy.initialStates.size(), this->automaton.initialStates.size());

            // Check that the copy is not the same object as the original
            BOOST_CHECK(&copy != &this->automaton);

            // Check that the states in the copy are not the same objects as the states in the original
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                BOOST_CHECK(copy.states[i].get() != this->automaton.states[i].get());
                BOOST_CHECK_EQUAL(copy.states[i]->isMatch, this->automaton.states[i]->isMatch);
            }

            // Check that the initial states in the copy point to the correct copied states
            for (size_t i = 0; i < this->automaton.initialStates.size(); ++i) {
                // Find the index of the initial state in the original automaton
                size_t originalIndex = 0;
                for (; originalIndex < this->automaton.states.size(); ++originalIndex) {
                    if (this->automaton.states[originalIndex] == this->automaton.initialStates[i]) {
                        break;
                    }
                }
                BOOST_CHECK_LT(originalIndex, this->automaton.states.size());

                // Check that the initial state in the copy points to the corresponding state
                BOOST_CHECK(copy.initialStates[i] == copy.states[originalIndex]);
            }

            // Check that the transitions in the copy point to the correct copied states
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                const auto& originalState = this->automaton.states[i];
                const auto& copiedState = copy.states[i];

                // Check that the transitions map has the same keys
                BOOST_CHECK_EQUAL(originalState->next.size(), copiedState->next.size());

                for (const auto& [action, originalTransitions] : originalState->next) {
                    // Check that the action exists in the copied state
                    BOOST_CHECK(copiedState->next.find(action) != copiedState->next.end());

                    const auto& copiedTransitions = copiedState->next.at(action);
                    BOOST_CHECK_EQUAL(originalTransitions.size(), copiedTransitions.size());

                    // Check each transition
                    for (size_t j = 0; j < originalTransitions.size(); ++j) {
                        const auto& originalTransition = originalTransitions[j];
                        const auto& copiedTransition = copiedTransitions[j];

                        // Check that the transition properties are the same
                        BOOST_CHECK_EQUAL(originalTransition.stringConstraints.size(), copiedTransition.stringConstraints.size());
                        BOOST_CHECK_EQUAL(originalTransition.numConstraints.size(), copiedTransition.numConstraints.size());
                        BOOST_CHECK_EQUAL(originalTransition.resetVars.size(), copiedTransition.resetVars.size());
                        BOOST_CHECK_EQUAL(originalTransition.guard.size(), copiedTransition.guard.size());

                        // Check that the target state is correctly mapped
                        auto originalTarget = originalTransition.target.lock();
                        auto copiedTarget = copiedTransition.target.lock();

                        BOOST_CHECK(originalTarget != copiedTarget);

                        // Find the index of the target state in the original automaton
                        size_t targetIndex = 0;
                        for (; targetIndex < this->automaton.states.size(); ++targetIndex) {
                            if (this->automaton.states[targetIndex] == originalTarget) {
                                break;
                            }
                        }
                        BOOST_CHECK_LT(targetIndex, this->automaton.states.size());

                        // Check that the target state in the copy points to the corresponding state
                        BOOST_CHECK(copiedTarget == copy.states[targetIndex]);
                    }
                }
            }

            // Check that the TimedAutomaton specific members are copied correctly
            BOOST_CHECK_EQUAL(copy.stringVariableSize, this->automaton.stringVariableSize);
            BOOST_CHECK_EQUAL(copy.numberVariableSize, this->automaton.numberVariableSize);
            BOOST_CHECK_EQUAL(copy.clockVariableSize, this->automaton.clockVariableSize);
        }

        BOOST_FIXTURE_TEST_CASE(ModifyAfterCopyTest, DataParametricCopy) {
            // Create a deep copy of the automaton
            auto copy = this->automaton.deepCopy();

            // Modify the original automaton
            this->automaton.states[0]->isMatch = true;
            this->automaton.clockVariableSize = 99;

            // Check that the copy is not affected by the changes to the original
            BOOST_CHECK(!copy.states[0]->isMatch);
            BOOST_CHECK_EQUAL(copy.clockVariableSize, 1);
        }

        BOOST_FIXTURE_TEST_CASE(ModifyCopyTest, DataParametricCopy) {
            // Create a deep copy of the automaton
            auto copy = this->automaton.deepCopy();

            // Modify the copy
            copy.states[0]->isMatch = true;
            copy.clockVariableSize = 99;

            // Add a new transition in the copy
            auto& transitions = copy.states[0]->next[0];
            auto newTransition = transitions[0]; // Clone an existing transition
            transitions.push_back(newTransition);

            // Check that the original is not affected by the changes to the copy
            BOOST_CHECK(!this->automaton.states[0]->isMatch);
            BOOST_CHECK_EQUAL(this->automaton.clockVariableSize, 1);
            BOOST_CHECK_EQUAL(this->automaton.states[0]->next[0].size(), 2); // Original should still have 2 transitions
        }

        BOOST_FIXTURE_TEST_CASE(WithdrawAutomatonTest, DataParametricWithdrawFixture) {
            // Create a deep copy of the withdraw automaton
            auto copy = this->automaton.deepCopy();

            // Check that the copy has the same number of states and initial states
            BOOST_CHECK_EQUAL(copy.states.size(), this->automaton.states.size());
            BOOST_CHECK_EQUAL(copy.initialStates.size(), this->automaton.initialStates.size());

            // Check that the copy is not the same object as the original
            BOOST_CHECK(&copy != &this->automaton);

            // Check that the states in the copy are not the same objects as the states in the original
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                BOOST_CHECK(copy.states[i].get() != this->automaton.states[i].get());
                BOOST_CHECK_EQUAL(copy.states[i]->isMatch, this->automaton.states[i]->isMatch);
            }

            // Check that the transitions in the copy point to the correct copied states
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                const auto& originalState = this->automaton.states[i];
                const auto& copiedState = copy.states[i];

                // Check that the transitions map has the same keys
                BOOST_CHECK_EQUAL(originalState->next.size(), copiedState->next.size());

                for (const auto& [action, originalTransitions] : originalState->next) {
                    const auto& copiedTransitions = copiedState->next.at(action);
                    BOOST_CHECK_EQUAL(originalTransitions.size(), copiedTransitions.size());
                }
            }

            // Modify the original automaton
            this->automaton.states[1]->isMatch = true;
            this->automaton.stringVariableSize = 99;

            // Check that the copy is not affected by the changes to the original
            BOOST_CHECK(!copy.states[1]->isMatch);
            BOOST_CHECK_EQUAL(copy.stringVariableSize, 1);
        }

    BOOST_AUTO_TEST_SUITE_END() // DataParametric

    BOOST_AUTO_TEST_SUITE(Parametric)

        BOOST_FIXTURE_TEST_CASE(BasicDeepCopyTest, ParametricCopy) {
            // Create a deep copy of the automaton
            auto copy = this->automaton.deepCopy();

            // Check that the copy has the same number of states and initial states
            BOOST_CHECK_EQUAL(copy.states.size(), this->automaton.states.size());
            BOOST_CHECK_EQUAL(copy.initialStates.size(), this->automaton.initialStates.size());

            // Check that the copy is not the same object as the original
            BOOST_CHECK(&copy != &this->automaton);

            // Check that the states in the copy are not the same objects as the states in the original
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                BOOST_CHECK(copy.states[i].get() != this->automaton.states[i].get());
                BOOST_CHECK_EQUAL(copy.states[i]->isMatch, this->automaton.states[i]->isMatch);
            }

            // Check that the initial states in the copy point to the correct copied states
            for (size_t i = 0; i < this->automaton.initialStates.size(); ++i) {
                // Find the index of the initial state in the original automaton
                size_t originalIndex = 0;
                for (; originalIndex < this->automaton.states.size(); ++originalIndex) {
                    if (this->automaton.states[originalIndex] == this->automaton.initialStates[i]) {
                        break;
                    }
                }
                BOOST_CHECK_LT(originalIndex, this->automaton.states.size());

                // Check that the initial state in the copy points to the corresponding state
                BOOST_CHECK(copy.initialStates[i] == copy.states[originalIndex]);
            }

            // Check that the transitions in the copy point to the correct copied states
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                const auto& originalState = this->automaton.states[i];
                const auto& copiedState = copy.states[i];

                // Check that the transitions map has the same keys
                BOOST_CHECK_EQUAL(originalState->next.size(), copiedState->next.size());

                for (const auto& [action, originalTransitions] : originalState->next) {
                    // Check that the action exists in the copied state
                    BOOST_CHECK(copiedState->next.find(action) != copiedState->next.end());

                    const auto& copiedTransitions = copiedState->next.at(action);
                    BOOST_CHECK_EQUAL(originalTransitions.size(), copiedTransitions.size());

                    // Check each transition
                    for (size_t j = 0; j < originalTransitions.size(); ++j) {
                        const auto& originalTransition = originalTransitions[j];
                        const auto& copiedTransition = copiedTransitions[j];

                        // Check that the transition properties are the same
                        BOOST_CHECK_EQUAL(originalTransition.stringConstraints.size(), copiedTransition.stringConstraints.size());
                        BOOST_CHECK_EQUAL(originalTransition.numConstraints.size(), copiedTransition.numConstraints.size());
                        BOOST_CHECK_EQUAL(originalTransition.resetVars.size(), copiedTransition.resetVars.size());

                        // Check that the parametric timing constraint is copied correctly
                        // We can't directly compare the constraints, but we can check they're not the same object
                        BOOST_CHECK(&originalTransition.guard != &copiedTransition.guard);

                        // Check that the target state is correctly mapped
                        auto originalTarget = originalTransition.target.lock();
                        auto copiedTarget = copiedTransition.target.lock();

                        BOOST_CHECK(originalTarget != copiedTarget);

                        // Find the index of the target state in the original automaton
                        size_t targetIndex = 0;
                        for (; targetIndex < this->automaton.states.size(); ++targetIndex) {
                            if (this->automaton.states[targetIndex] == originalTarget) {
                                break;
                            }
                        }
                        BOOST_CHECK_LT(targetIndex, this->automaton.states.size());

                        // Check that the target state in the copy points to the corresponding state
                        BOOST_CHECK(copiedTarget == copy.states[targetIndex]);
                    }
                }
            }

            // Check that the TimedAutomaton specific members are copied correctly
            BOOST_CHECK_EQUAL(copy.stringVariableSize, this->automaton.stringVariableSize);
            BOOST_CHECK_EQUAL(copy.numberVariableSize, this->automaton.numberVariableSize);
            BOOST_CHECK_EQUAL(copy.clockVariableSize, this->automaton.clockVariableSize);
            BOOST_CHECK_EQUAL(copy.parameterSize, this->automaton.parameterSize);
        }

        BOOST_FIXTURE_TEST_CASE(ModifyAfterCopyTest, ParametricCopy) {
            // Create a deep copy of the automaton
            auto copy = this->automaton.deepCopy();

            // Modify the original automaton
            this->automaton.states[0]->isMatch = true;
            this->automaton.clockVariableSize = 99;
            this->automaton.parameterSize = 5;

            // Check that the copy is not affected by the changes to the original
            BOOST_CHECK(!copy.states[0]->isMatch);
            BOOST_CHECK_EQUAL(copy.clockVariableSize, 1);
            BOOST_CHECK_EQUAL(copy.parameterSize, 0);
        }

        BOOST_FIXTURE_TEST_CASE(ModifyCopyTest, ParametricCopy) {
            // Create a deep copy of the automaton
            auto copy = this->automaton.deepCopy();

            // Modify the copy
            copy.states[0]->isMatch = true;
            copy.clockVariableSize = 99;
            copy.parameterSize = 5;

            // Add a new transition in the copy
            auto& transitions = copy.states[0]->next[0];
            auto newTransition = transitions[0]; // Clone an existing transition
            transitions.push_back(newTransition);

            // Check that the original is not affected by the changes to the copy
            BOOST_CHECK(!this->automaton.states[0]->isMatch);
            BOOST_CHECK_EQUAL(this->automaton.clockVariableSize, 1);
            BOOST_CHECK_EQUAL(this->automaton.parameterSize, 0);
            BOOST_CHECK_EQUAL(this->automaton.states[0]->next[0].size(), 2); // Original should still have 2 transitions
        }

        BOOST_FIXTURE_TEST_CASE(WithdrawAutomatonTest, ParametricWithdrawFixture) {
            // Create a deep copy of the withdraw automaton
            auto copy = this->automaton.deepCopy();

            // Check that the copy has the same number of states and initial states
            BOOST_CHECK_EQUAL(copy.states.size(), this->automaton.states.size());
            BOOST_CHECK_EQUAL(copy.initialStates.size(), this->automaton.initialStates.size());

            // Check that the copy is not the same object as the original
            BOOST_CHECK(&copy != &this->automaton);

            // Check that the states in the copy are not the same objects as the states in the original
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                BOOST_CHECK(copy.states[i].get() != this->automaton.states[i].get());
                BOOST_CHECK_EQUAL(copy.states[i]->isMatch, this->automaton.states[i]->isMatch);
            }

            // Check that the transitions in the copy point to the correct copied states
            for (size_t i = 0; i < this->automaton.states.size(); ++i) {
                const auto& originalState = this->automaton.states[i];
                const auto& copiedState = copy.states[i];

                // Check that the transitions map has the same keys
                BOOST_CHECK_EQUAL(originalState->next.size(), copiedState->next.size());

                for (const auto& [action, originalTransitions] : originalState->next) {
                    const auto& copiedTransitions = copiedState->next.at(action);
                    BOOST_CHECK_EQUAL(originalTransitions.size(), copiedTransitions.size());
                }
            }

            // Modify the original automaton
            this->automaton.states[1]->isMatch = true;
            this->automaton.stringVariableSize = 99;
            this->automaton.parameterSize = 5;

            // Check that the copy is not affected by the changes to the original
            BOOST_CHECK(!copy.states[1]->isMatch);
            BOOST_CHECK_EQUAL(copy.stringVariableSize, 1);
            BOOST_CHECK_EQUAL(copy.parameterSize, 0);
        }

    BOOST_AUTO_TEST_SUITE_END() // Parametric

BOOST_AUTO_TEST_SUITE_END() // AutomatonDeepCopyTest
